use godot::prelude::*;
use pulldown_cmark::Event::*;
use pulldown_cmark::{Alignment, CowStr, Event, LinkType, Options, Parser, Tag, TagEnd};
use pulldown_cmark_escape::{escape_href, escape_html, StrWrite};
use std::collections::HashMap;
use std::io::{self};

struct GodotMarkdown;

#[gdextension]
unsafe impl ExtensionLibrary for GodotMarkdown {}

#[derive(GodotClass)]
#[class(init,base=Node)]
struct Cmark {
    base: Base<Node>,
    #[init(default = 16)]
    #[export]
    font_size: i64,
    #[init(default = 2.0)]
    #[export]
    h1_size: f64,
    #[init(default = 1.5)]
    #[export]
    h2_size: f64,
    #[init(default = 1.17)]
    #[export]
    h3_size: f64,
    #[init(default = 1.0)]
    #[export]
    h4_size: f64,
    #[init(default = 0.83)]
    #[export]
    h5_size: f64,
    #[init(default = 0.67)]
    #[export]
    h6_size: f64,
}

#[godot_api]
impl Cmark {
    #[func]
    fn convert_markdown(&mut self, markdown: GString) -> GString {
        let markdown_input: &str = &markdown.to_string();
        let mut options = Options::empty();
        options.insert(Options::ENABLE_TASKLISTS);
        options.insert(Options::ENABLE_STRIKETHROUGH);
        options.insert(Options::ENABLE_TABLES);
        let parser = Parser::new_ext(markdown_input, options);
        let mut richtext_output: String = String::with_capacity(markdown_input.len() * 3 / 2);
        push_richtext(&mut richtext_output, parser);
        GString::from(richtext_output)
    }
}

enum TableState {
    Head,
    Body,
}

fn push_richtext<'a, I>(s: &mut String, iter: I)
where
    I: Iterator<Item = Event<'a>>,
{
    RichTextWriter::new(iter, s).run().unwrap();
}

struct RichTextWriter<'a, I, W> {
    iter: I,
    writer: W,
    /// Whether or not the last write wrote a newline.
    end_newline: bool,
    table_state: TableState,
    table_alignments: Vec<Alignment>,
    table_cell_index: usize,
    numbers: HashMap<CowStr<'a>, usize>,
}

impl<'a, I, W> RichTextWriter<'a, I, W>
where
    I: Iterator<Item = Event<'a>>,
    W: StrWrite,
{
    fn new(iter: I, writer: W) -> Self {
        Self {
            iter,
            writer,
            end_newline: true,
            table_state: TableState::Head,
            table_alignments: vec![],
            table_cell_index: 0,
            numbers: HashMap::new(),
        }
    }

    fn write_newline(&mut self) -> io::Result<()> {
        self.end_newline = true;
        self.writer.write_str("\n")
    }

    /// Writes a buffer, and tracks whether or not a newline was written.
    #[inline]
    fn write(&mut self, s: &str) -> io::Result<()> {
        self.writer.write_str(s)?;

        if !s.is_empty() {
            self.end_newline = s.ends_with('\n');
        }
        Ok(())
    }

    fn run(mut self) -> io::Result<()> {
        while let Some(event) = self.iter.next() {
            match event {
                Start(tag) => {
                    self.start_tag(tag)?;
                }
                End(tag) => {
                    self.end_tag(tag)?;
                }
                Text(text) => {
                    self.write(&text)?;
                }
                Code(text) => {
                    self.write("[code]")?;
                    self.write(&text)?;
                    self.write("[/code]")?;
                }
                Html(html) | InlineHtml(html) => {
                    self.write(&html)?;
                }
                SoftBreak => {
                    self.write_newline()?;
                }
                HardBreak => {
                    self.write_newline()?;
                    self.write_newline()?;
                }
                Rule => {
                    if self.end_newline {
                        self.write("\n\n")?;
                    } else {
                        self.write("\n\n\n")?;
                    }
                }
                FootnoteReference(name) => {
                    let len = self.numbers.len() + 1;
                    self.write("<sup class=\"footnote-reference\"><a href=\"#")?;
                    escape_html(&mut self.writer, &name)?;
                    self.write("\">")?;
                    let number = *self.numbers.entry(name).or_insert(len);
                    write!(&mut self.writer, "{}", number)?;
                    self.write("</a></sup>")?;
                }
                TaskListMarker(true) => {
                    self.write("☑ ")?;
                }
                TaskListMarker(false) => {
                    self.write("🔲 ")?;
                }
            }
        }
        Ok(())
    }

    /// Writes the start of an HTML tag.
    fn start_tag(&mut self, tag: Tag<'a>) -> io::Result<()> {
        match tag {
            Tag::Paragraph => {
                if self.end_newline {
                    self.write("")
                } else {
                    self.write("\n")
                }
            }
            Tag::Heading {
                level,
                id: _,
                classes: _,
                attrs: _,
            } => {
                if self.end_newline {
                    self.end_newline = false;
                    self.write("[{")?;
                } else {
                    self.write("\n[{")?;
                }
                write!(&mut self.writer, "font_size={}", level)?;
                self.write("}]")
            }
            Tag::Table(alignments) => {
                self.table_alignments = alignments;
                self.write("[table=")?;
                write!(&mut self.writer, "{}", self.table_alignments.len())?;
                self.write("]")
            }
            Tag::TableHead => {
                self.table_state = TableState::Head;
                self.table_cell_index = 0;
                self.write("")
            }
            Tag::TableRow => {
                self.table_cell_index = 0;
                self.write("")
            }
            Tag::TableCell => {
                match self.table_state {
                    TableState::Head => {
                        self.write("[cell border=white]")?;
                    }
                    TableState::Body => {
                        self.write("[cell border=white]")?;
                    }
                }
                match self.table_alignments.get(self.table_cell_index) {
                    Some(&Alignment::Left) => self.write(""),
                    Some(&Alignment::Center) => self.write(""),
                    Some(&Alignment::Right) => self.write(""),
                    _ => self.write(""),
                }
            }
            Tag::BlockQuote => {
                if self.end_newline {
                    self.write("[p][indent]\n")
                } else {
                    self.write("\n[p][indent]\n")
                }
            }
            Tag::CodeBlock(info) => {
                if !self.end_newline {
                    self.write_newline()?;
                }
                self.write("[p][code]")
            }
            Tag::List(Some(num)) => {
                if self.end_newline {
                    self.write("[ol]\n")
                } else {
                    self.write("\n[ol]\n")
                }
            }
            Tag::List(None) => {
                if self.end_newline {
                    self.write("[ul]\n")
                } else {
                    self.write("\n[ul]\n")
                }
            }
            Tag::Item => {
                if !self.end_newline {
                    self.write("\n")
                } else {
                    self.write("")
                }
            }
            Tag::Emphasis => self.write("[i]"),
            Tag::Strong => self.write("[b]"),
            Tag::Strikethrough => self.write("[s]"),
            Tag::Link {
                link_type: LinkType::Email,
                dest_url,
                title,
                id: _,
            } => {
                self.write("[url=\"mailto:")?;
                escape_href(&mut self.writer, &dest_url)?;
                self.write("\"]")
            }
            Tag::Link {
                link_type: _,
                dest_url,
                title,
                id: _,
            } => {
                self.write("[url=\"")?;
                escape_href(&mut self.writer, &dest_url)?;
                self.write("\"]")
            }
            Tag::Image {
                link_type: _,
                dest_url,
                title,
                id: _,
            } => {
                self.write("[img]")?;
                escape_href(&mut self.writer, &dest_url)?;
                self.write("[/img]")
            }
            Tag::FootnoteDefinition(name) => {
                if self.end_newline {
                    self.write("<div class=\"footnote-definition\" id=\"")?;
                } else {
                    self.write("\n<div class=\"footnote-definition\" id=\"")?;
                }
                escape_html(&mut self.writer, &*name)?;
                self.write("\"><sup class=\"footnote-definition-label\">")?;
                let len = self.numbers.len() + 1;
                let number = *self.numbers.entry(name).or_insert(len);
                write!(&mut self.writer, "{}", number)?;
                self.write("</sup>")
            }
            Tag::HtmlBlock => Ok(()),
            Tag::MetadataBlock(_) => Ok(()),
        }
    }

    fn end_tag(&mut self, tag: TagEnd) -> io::Result<()> {
        match tag {
            TagEnd::Paragraph => {
                self.write("\n")?;
            }
            TagEnd::Heading(level) => {
                self.write("[/font_size]\n")?;
            }
            TagEnd::Table => {
                self.write("[/table]\n")?;
            }
            TagEnd::TableHead => {
                self.write("\n")?;
                self.table_state = TableState::Body;
            }
            TagEnd::TableRow => {
                self.write("\n")?;
            }
            TagEnd::TableCell => {
                match self.table_state {
                    TableState::Head => {
                        self.write("[/cell]")?;
                    }
                    TableState::Body => {
                        self.write("[/cell]")?;
                    }
                }
                self.table_cell_index += 1;
            }
            TagEnd::BlockQuote => {
                self.write("[/indent][/p]\n")?;
            }
            TagEnd::CodeBlock => {
                self.write("[/code][/p]\n")?;
            }
            TagEnd::List(true) => {
                self.write("[/ol]\n")?;
            }
            TagEnd::List(false) => {
                self.write("[/ul]\n")?;
            }
            TagEnd::Item => {
                self.write("\n")?;
            }
            TagEnd::Emphasis => {
                self.write("[/i]")?;
            }
            TagEnd::Strong => {
                self.write("[/b]")?;
            }
            TagEnd::Strikethrough => {
                self.write("[/s]")?;
            }
            TagEnd::Link => {
                self.write("[/url]")?;
            }
            TagEnd::Image => (), // shouldn't happen, handled in start
            TagEnd::FootnoteDefinition => {
                self.write("</div>\n")?;
            }
            TagEnd::HtmlBlock => {}
            TagEnd::MetadataBlock(_) => {}
        }
        Ok(())
    }
}

#[no_mangle]
pub extern "C" fn gdmarkdowntest() {
    println!("running main");
    let markdown_input: &str = "Hello world, this is a ~~complicated~~ *very simple* example.";
    println!("Parsing the following markdown string:\n{}", markdown_input);
    let mut options = Options::empty();
    options.insert(Options::ENABLE_TASKLISTS);
    options.insert(Options::ENABLE_STRIKETHROUGH);
    let parser = Parser::new_ext(markdown_input, options);
    let mut richtext_output: String = String::with_capacity(markdown_input.len() * 3 / 2);
    push_richtext(&mut richtext_output, parser);
    println!("\nBBCode output:\n{}", &richtext_output);
}
