use godot::prelude::*;
use rushdown::ast::*;
use rushdown::matches_kind;
use rushdown::parser::GfmOptions;
use rushdown::parser::Parser as RDParser;
use rushdown::text::BasicReader;

#[derive(GodotClass)]
#[class(init, base=Resource)]
struct KanbanDocument {
    base: Base<Resource>,
    root: NodeRef,
    arena: Arena,
    #[var]
    source: GString,
    #[var]
    projects: Array<Gd<KanbanProject>>,
    #[var]
    status_order: Array<StringName>,
}

#[derive(GodotClass)]
#[class(init, base=Resource)]
struct KanbanProject {
    base: Base<Resource>,
    node_ref: NodeRef,
    #[var]
    name: GString,
    #[var]
    tasks: Array<Variant>,
    #[var]
    statuses: Array<StringName>,
}

#[derive(GodotClass)]
#[class(init, base=Resource)]
struct KanbanTask {
    base: Base<Resource>,
    node_ref: NodeRef,
    #[var]
    done: bool,
    #[var]
    text: GString,
    #[var]
    status: StringName,
}

#[derive(Debug, Clone)]
struct WalkerError;

impl std::fmt::Display for WalkerError {
    fn fmt(&self, f: &mut std::fmt::Formatter) -> std::fmt::Result {
        write!(f, "WalkerError, very hilfreich, wtf rust ist arg explizit")
    }
}

impl std::error::Error for WalkerError {}

#[derive(Eq, PartialEq, Hash)]
enum StatusLevel {
    Backlog,
    Todo,
    Doing,
    Done,
    Other,
}

#[derive(Eq, PartialEq, Hash)]
struct Status {
    level: StatusLevel,
    name: StringName,
}

impl Status {
    fn enum_index(&self) -> u8 {
        match self.level {
            StatusLevel::Backlog => 0,
            StatusLevel::Todo => 1,
            StatusLevel::Doing => 2,
            StatusLevel::Other => 3,
            StatusLevel::Done => 4,
        }
    }
}

impl From<StringName> for Status {
    fn from(name: StringName) -> Self {
        let level: StatusLevel;
        let lname = name.to_lower();
        if lname.contains("todo") | lname.contains("to do") {
            level = StatusLevel::Todo;
        } else if lname.contains("done") | lname.contains("complete") {
            level = StatusLevel::Done;
        } else if lname.contains("doing") | lname.contains("progress") {
            level = StatusLevel::Doing;
        } else if lname.contains("backlog") {
            level = StatusLevel::Backlog;
        } else {
            level = StatusLevel::Other;
        }
        Status { level, name }
    }
}

impl Ord for Status {
    fn cmp(&self, other: &Self) -> std::cmp::Ordering {
        self.enum_index().cmp(&other.enum_index())
    }
}

impl PartialOrd for Status {
    fn partial_cmp(&self, other: &Self) -> Option<std::cmp::Ordering> {
        Some(self.cmp(&other))
    }
}

#[godot_api]
impl KanbanDocument {
    #[func]
    fn from_markdown_text(text: GString) -> Gd<Self> {
        Gd::from_init_fn(|base| {
            let mut doc = Self {
                base,
                root: NodeRef::default(),
                arena: Arena::default(),
                source: text,
                projects: Array::default(),
                status_order: Array::default(),
            };
            doc.find_kanbanizable_tasklist();
            let mut all_statuses = std::collections::HashSet::<Status>::new();
            for project in doc.projects.iter_shared() {
                for status in project.bind().statuses.iter_shared() {
                    all_statuses.insert(Status::from(status));
                }
            }
            let mut all_statuses_ordered = all_statuses.into_iter().collect::<Vec<_>>();
            all_statuses_ordered.sort();
            for status in all_statuses_ordered {
                doc.status_order.push(&status.name);
            }
            for mut project in doc.projects.iter_shared() {
                project.bind_mut().statuses = doc.status_order.clone();
            }
            doc
        })
    }

    fn find_kanbanizable_tasklist(&mut self) {
        let parser = RDParser::with_extensions(
            rushdown::parser::Options::default(),
            rushdown::parser::gfm(GfmOptions::default()),
        );
        let source = &self.source.to_string();
        let mut reader = BasicReader::new(source);
        (self.arena, self.root) = parser.parse(&mut reader);
        let mut project_list = Vec::new();
        walk(&self.arena, self.root, &mut |arena: &Arena,
                                           node_ref: NodeRef,
                                           entering: bool|
         -> Result<
            WalkStatus,
            WalkerError,
        > {
            if entering {
                if !matches_kind!(arena, node_ref, ListItem) {
                    return Ok(WalkStatus::Continue);
                }
                if !Self::is_project(&self.arena, node_ref) {
                    return Ok(WalkStatus::Continue);
                }
                project_list.push(node_ref);
            }
            Ok(WalkStatus::Continue)
        })
        .ok();
        if project_list.is_empty() {
            return;
        }
        project_list.iter().for_each(|&p_ref| {
            let project = KanbanProject::from_ast(&self.arena, p_ref, source);
            self.projects.push(&project);
        });
    }

    fn get_first_child_list(arena: &Arena, list_ref: NodeRef) -> Option<NodeRef> {
        // find first item
        let item = arena[list_ref].first_child()?;
        // find first list
        let mut children = arena[item].children_mut(arena);
        children.find(|&s| matches_kind!(arena, s, List))
    }

    fn is_project(arena: &Arena, list_item_ref: NodeRef) -> bool {
        let mut children = arena[list_item_ref].children_mut(arena);
        let Some(status_list) = children.find(|&s| matches_kind!(arena, s, List)) else {
            return false;
        };
        if let Some(status_ref) = arena[status_list].first_child() {
            if let KindData::ListItem(status) = arena[status_ref].kind_data() {
                if status.is_task() {
                    return false;
                };
            };
        };
        let Some(task_list) = Self::get_first_child_list(arena, status_list) else {
            return false;
        };
        let Some(item_ref) = arena[task_list].first_child() else {
            return false;
        };
        let KindData::ListItem(task) = arena[item_ref].kind_data() else {
            return false;
        };
        task.is_task()
    }
}

#[godot_api]
impl KanbanProject {
    fn from_ast(arena: &Arena, project_ref: NodeRef, source: &String) -> Gd<Self> {
        Gd::from_init_fn(|base| {
            let mut project = Self {
                base: base,
                name: GString::default(),
                node_ref: project_ref,
                tasks: Array::default(),
                statuses: Array::default(),
            };
            if let Some(para_ref) = arena[project.node_ref].first_child() {
                if let Some(text) = paragraph_to_text(para_ref, arena, source) {
                    project.name = GString::from(&text);
                }
            }
            if let Some(status_list_ref) = arena[project.node_ref]
                .children(arena)
                .find(|&s| matches_kind!(arena, s, List))
            {
                for status_item_ref in arena[status_list_ref].children(arena) {
                    let Some(para_ref) = arena[status_item_ref].first_child() else {
                        break;
                    };
                    let Some(status_text) = paragraph_to_text(para_ref, arena, source) else {
                        break;
                    };
                    project.statuses.push(&status_text);
                    let status = StringName::from(&status_text);
                    let Some(task_list_ref) = arena[status_item_ref]
                        .children(arena)
                        .find(|&s| matches_kind!(arena, s, List))
                    else {
                        break;
                    };
                    for task_item_ref in arena[task_list_ref].children(arena) {
                        let task =
                            KanbanTask::from_ast(status.clone(), arena, task_item_ref, source);
                        project.tasks.push(&task);
                    }
                }
            };
            project
        })
    }
}

fn paragraph_to_text(para_ref: NodeRef, arena: &Arena, source: &String) -> Option<String> {
    if !matches_kind!(arena, para_ref, Paragraph) {
        return None;
    }
    let mut ret: String = "".to_owned();
    let _ = walk(arena, para_ref, &mut |arena: &Arena,
        node_ref: NodeRef,
        entering: bool|
        -> Result<
        WalkStatus,
        WalkerError,
        > {
            if entering {
                if let KindData::Text(text) = arena[node_ref].kind_data() {
                    ret.push_str(text.str(source));
                    if text.has_qualifiers(TextQualifier::SOFT_LINE_BREAK ) || text.has_qualifiers(TextQualifier::HARD_LINE_BREAK) {
                        ret.push_str(" ");
                    }
                };
            };
            Ok(WalkStatus::Continue)
        });

    Some(ret)
}

#[godot_api]
impl KanbanTask {
    fn from_ast(status: StringName, arena: &Arena, task_ref: NodeRef, source: &String) -> Gd<Self> {
        Gd::from_init_fn(|base| {
            let mut task = Self {
                base: base,
                node_ref: task_ref,
                text: GString::default(),
                status: status,
                done: false,
            };
            if let Some(para_ref) = arena[task_ref].first_child() {
                if let Some(text) = paragraph_to_text(para_ref, arena, source) {
                    task.text = GString::from(&text);
                }
            }
            if let KindData::ListItem(list_item) = arena[task_ref].kind_data() {
                if let Some(task_item) = list_item.task() {
                    task.done = task_item == Task::Completed;
                }
            }
            task
        })
    }
}

#[cfg(test)]
mod test {
    use godot::{
        classes::Resource,
        obj::{Base, Gd, NewGd},
        prelude::godot_api,
    };

    use crate::kanban::KanbanDocument;

    #[test]
    fn print_found_lists() {
        let source = r#"
* project 1
  * doing
    * [ ] task 1 
  * to do
    * [ ] task 2 
  * backlog
    * [ ] task 3
    * [ ] task 4
  * done
    * [x] task 5
* project 2
  * doing
    * [ ] aufgabe 1 
  * to do
    * [ ] aufgabe 2 
  * backlog
    * [ ] aufgabe 3
    * [ ] aufgabe 4
  * done
    * [x] aufgabe 5

paragraph to break list

* project after break and with lots of text
  and even a newline because lmao
  * doing
    * [ ] task 1 
  * to do
    * [ ] task 2 
  * backlog
    * [ ] task 3
    * [ ] task 4
  * done
    * [x] task 5
        "#;
        let k = KanbanDocument::init();
        k.find_kanbanizable_tasklist(source);
    }
}
