use godot::prelude::Node as GDNode;
use godot::prelude::*;
use rushdown::parser::Parser as RDParser;
use rushdown::parser::GfmOptions;
use rushdown::ast::Node as RDNode;
use rushdown::ast::*;
use rushdown::text::{BasicReader};
use rushdown::matches_kind;

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

#[godot_api]
impl KanbanDocument {

    #[func]
    fn from_markdown_text(text: GString) -> Gd<Self> {
        Gd::from_init_fn(|base| {
            let mut doc = Self {base,
               root: NodeRef::default(),
               arena: Arena::default(),
               source: text,
               projects: Array::default(),
            };
            doc.find_kanbanizable_tasklist();
            doc
        })
    }

    fn find_kanbanizable_tasklist(&mut self) {
        let parser = RDParser::with_extensions(rushdown::parser::Options::default(), rushdown::parser::gfm(GfmOptions::default()));
        let source = &self.source.to_string();
        let mut reader = BasicReader::new(source);
        (self.arena, self.root) = parser.parse(&mut reader);
        let mut project_list = Vec::new();
        walk(&self.arena, self.root, &mut |arena: &Arena,
            node_ref: NodeRef,
            entering: bool| -> Result<WalkStatus, WalkerError> {
                if entering {
                    if !matches_kind!(arena, node_ref, ListItem) {
                        return Ok(WalkStatus::Continue)
                    }
                    if !Self::is_project(&self.arena, node_ref) {
                        return Ok(WalkStatus::Continue)
                    }
                    project_list.push(node_ref);
            }
                Ok(WalkStatus::Continue)
            }).ok();
        if project_list.is_empty() {
            return
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
        let Some(status_list) = children.find(|&s| matches_kind!(arena, s, List)) else { return false };
        let Some(task_list) = Self::get_first_child_list(arena, status_list) else { return false };
        let Some(item_ref) = arena[task_list].first_child() else { return false };
        let KindData::ListItem(task) = arena[item_ref].kind_data() else { return false };
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
            match arena[project.node_ref].first_child() {
                None => (),
                Some(para_ref) => {
                    let mut name: String = "".to_owned();
                    match arena[para_ref].kind_data() {
                        KindData::Paragraph(_) => {
                            arena[para_ref].children(&arena).for_each(|c| {
                                match arena[c].kind_data() {
                                    KindData::Text(text) => name.push_str(text.str(source)),
                                    _ => ()
                                }
                            });
                            project.name = name.into();
                        }
                        _ => ()
                    }
                }
            }
            project
        })
    }

}

#[cfg(test)]
mod test {
    use godot::{classes::Resource, obj::{Base, Gd, NewGd}};

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
