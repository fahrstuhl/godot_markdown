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
    projects: Array<Variant>,
}

#[derive(GodotClass)]
#[class(init, base=Resource)]
struct KanbanProject {
    node_ref: NodeRef,
    base: Base<Resource>,
    #[var]
    task: Array<Variant>,
    #[var]
    statuses: Array<StringName>,
}

#[derive(GodotClass)]
#[class(init, base=Resource)]
struct KanbanTask {
    node_ref: NodeRef,
    base: Base<Resource>,
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
    fn find_kanbanizable_tasklist(self, source: &str) {
        let parser = RDParser::with_extensions(rushdown::parser::Options::default(), rushdown::parser::gfm(GfmOptions::default()));
        let mut reader = BasicReader::new(source);
        let (arena, document_ref)  = parser.parse(&mut reader);
        let mut projects = Vec::new();
        walk(&arena, document_ref, &mut |arena: &Arena,
            node_ref: NodeRef,
            entering: bool| -> Result<WalkStatus, WalkerError> {
                if entering {
                    if !matches_kind!(arena, node_ref, List) {
                        return Ok(WalkStatus::Continue)
                    }
                    let status_list = Self::get_first_child_list(arena, node_ref);
                    if status_list.is_none() {
                        return Ok(WalkStatus::Continue)
                    }
                    let status_list = status_list.unwrap();
                    let task_list = Self::get_first_child_list(arena, status_list);
                    if task_list.is_none() {
                        return Ok(WalkStatus::Continue)
                    }
                    let task_list = task_list.unwrap();
                    let item_ref = arena[task_list].first_child();
                    if item_ref.is_none() {
                        return Ok(WalkStatus::Continue)
                    }
                    let item_ref = item_ref.unwrap();

                    match arena[item_ref].kind_data() {
                        KindData::ListItem(task) => {
                            if !task.is_task() {
                                return Ok(WalkStatus::Continue);
                            }
                            let project = arena[node_ref].first_child();
                            if project.is_none() {
                                return Ok(WalkStatus::Continue)
                            }
                            let project = project.unwrap();
                            projects.push(project);
                            println!("found project!");
                        }
                        _ => (),
                    }
                }
                Ok(WalkStatus::Continue)
            }).ok();
        if projects.is_empty() {
            println!("no projects found");
            return
        }
        projects.iter().for_each(|&p_ref| {
            match arena[p_ref].first_child() {
                None => (),
                Some(para_ref) => {
                    match arena[para_ref].kind_data() {
                        KindData::Paragraph(_) => {
                            print!("Project name: ");
                            arena[para_ref].children(&arena).for_each(|c| {
                                match arena[c].kind_data() {
                                    KindData::Text(text) => print!("{}", text.str(source)),
                                    _ => ()
                                }
                            });
                            println!();
                        }
                        _ => ()
                    }
                }
            }
        });
    }

    fn get_first_child_list(arena: &Arena, list_ref: NodeRef) -> Option<NodeRef> {
        // find first item
        let item = arena[list_ref].first_child()?;
        // find first list
        let mut children = arena[item].children_mut(arena);
        children.find(|&s| matches_kind!(arena, s, List))
    }
}

#[cfg(test)]
mod test {
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
        let k = KanbanDocument{};
        k.find_kanbanizable_tasklist(source);
    }
}
