#include "cmark.h"

#include <godot_cpp/core/class_db.hpp>

#include <godot_cpp/classes/global_constants.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

#include <godot_cpp/variant/char_string.hpp>
#include <godot_cpp/variant/string.hpp>

#include <string.h>
#include "cmark-gfm.h"
#include "cmark-gfm-core-extensions.h"
#include "table.h"
#ifndef _WINDOWS
#include "strikethrough.h"
#endif

const char *extension_names[] = {
    "autolink",
#ifndef _WINDOWS
    "strikethrough",
#endif
    "table",
    "tagfilter",
    "tasklist",
    NULL,
};

using namespace godot;

void Cmark::_bind_methods() {
	ClassDB::bind_method(D_METHOD("convert_markdown"), &Cmark::convert_markdown);
}

Cmark::Cmark() {
}

Cmark::~Cmark() {
}

String Cmark::convert_markdown(const String &markdown) {
    /* godot_string data; */
    /* data = api->godot_variant_as_string(p_args[0]); */

    /* godot_char_string parsed; */
    /* godot_string converted; */
    /* api->godot_string_new(&converted); */
    /* godot_variant ret; */
    /* parsed = api->godot_string_utf8(&data); */
    String converted = String();

    cmark_gfm_core_extensions_ensure_registered();
    CharString utf8 = markdown.utf8();
    const char* to_parse = utf8.get_data();
    cmark_parser * parser = cmark_parser_new(0);
    for (const char **it = extension_names; *it; ++it) {
        const char *extension_name = *it;
        cmark_syntax_extension *syntax_extension = cmark_find_syntax_extension(extension_name);
        if (!syntax_extension) {
            fprintf(stderr, "%s is not a valid syntax extension\n", extension_name);
            abort();
        }
        cmark_parser_attach_syntax_extension(parser, syntax_extension);
    }
    cmark_parser_feed(parser, to_parse, strlen(to_parse));
    cmark_node * document_root = cmark_parser_finish(parser);
    cmark_iter *iter = cmark_iter_new(document_root);
    cmark_event_type ev_type;
    cmark_node *parent;
    cmark_node *grandparent;
    int list_level = 0;
    while ((ev_type = cmark_iter_next(iter)) != CMARK_EVENT_DONE) {
        int content_handled = 0;
        cmark_node *cur = cmark_iter_get_node(iter);
        cmark_node_type cur_type = cmark_node_get_type(cur);
        const char* content = cmark_node_get_literal(cur);
        if (cur_type == CMARK_NODE_DOCUMENT){
        }
        else if(cur_type == CMARK_NODE_BLOCK_QUOTE){
            if (ev_type == CMARK_EVENT_ENTER) {
            	converted = converted + String("[indent]");
            }
            else if (ev_type == CMARK_EVENT_EXIT) {
                converted = converted + "[/indent]";
            }
        }
        else if(cur_type == CMARK_NODE_LIST){
            if (ev_type == CMARK_EVENT_ENTER) {
                /* converted = converted + "<li>"; */
                list_level += 1;
            }
            else if (ev_type == CMARK_EVENT_EXIT) {
                /* converted = converted + "</li>"; */
                list_level -= 1;
            }
        }
        else if(cur_type == CMARK_NODE_ITEM){
            if (ev_type == CMARK_EVENT_ENTER) {
                if(list_level >= 1) {
                    converted = converted + "[indent]";
                }
                parent = cmark_node_parent(cur);
                char* tasklist_state = cmark_gfm_extensions_get_tasklist_state(cur);
                bool is_tasklist = strcmp(cmark_node_get_type_string(cur), "tasklist") == 0;
                if (parent != NULL
                        && cmark_node_get_type(parent) == CMARK_NODE_LIST
                        && cmark_node_get_list_type(parent) == CMARK_ORDERED_LIST) {
                    int list_start = cmark_node_get_list_start(parent);
                    converted = converted + String::num(list_start, 0);
                    converted = converted + ". ";
                    cmark_node_set_list_start(parent, list_start + 1);
                }
                else if (!is_tasklist) {
                    converted = converted + String::utf8("• ");
                }
                else if (strcmp(tasklist_state, "checked") == 0) {
                    converted = converted + String::utf8("🔲 ");
                }
                else if (strcmp(tasklist_state, "unchecked") == 0) {
                    converted = converted + String::utf8("☑ ");
                }
            }
            else if (ev_type == CMARK_EVENT_EXIT) {
                if(list_level >= 1) {
                    converted = converted + "[/indent]";
                }
            }
        }
        else if(cur_type == CMARK_NODE_CODE_BLOCK){
            if (ev_type == CMARK_EVENT_ENTER) {
                converted = converted + "[indent][code]";
                converted = converted + String::utf8(content);
                converted = converted + "[/code][/indent]";
                content_handled = 1;
            }
        }
        else if(cur_type == CMARK_NODE_HTML_BLOCK){
        }
        else if(cur_type == CMARK_NODE_CUSTOM_BLOCK){
        }
        else if(cur_type == CMARK_NODE_PARAGRAPH){
            parent = cmark_node_parent(cur);
            grandparent = cmark_node_parent(parent);
            int tight;
            int is_child_of_item = parent != NULL && cmark_node_get_type(parent) == CMARK_NODE_ITEM;
            int is_not_first_child_of_item;
            if (is_child_of_item) {
                is_not_first_child_of_item = cur != cmark_node_first_child(parent);
            }
            if (grandparent != NULL && cmark_node_get_type(grandparent) == CMARK_NODE_LIST) {
                tight = cmark_node_get_list_tight(grandparent);
            } else {
                tight = false;
            }
            if (ev_type == CMARK_EVENT_ENTER) {
                if (is_child_of_item && is_not_first_child_of_item) {
                    converted = converted + "  ";
                }
                /* converted = converted + "<p>"; */
            }
            else if (ev_type == CMARK_EVENT_EXIT) {
                /* converted = converted + "</p>"; */
                if (is_child_of_item) {
                    if (is_not_first_child_of_item) {
                        /* converted = converted + "[/indent]"; */
                    }
                    if (!tight) {
                        converted = converted + "\n\n";
                    }
                    else {
                        converted = converted + "\n";
                    }
                } else {
                    converted = converted + "\n\n";
                }
            }
        }
        else if(cur_type == CMARK_NODE_HEADING){
            if (ev_type == CMARK_EVENT_ENTER) {
                converted = converted + "[font_size={heading_";
                int heading_level = cmark_node_get_heading_level(cur);
                converted = converted + String::num(heading_level, 0);
                converted = converted + "}]";
            }
            else if (ev_type == CMARK_EVENT_EXIT) {
                converted = converted + "[/font_size]\n";
            }
        }
        else if(cur_type == CMARK_NODE_THEMATIC_BREAK){
            converted = converted + "---\n\n";
        }
        else if(cur_type == CMARK_NODE_FOOTNOTE_DEFINITION){
        }
        else if(cur_type == CMARK_NODE_TEXT){
        }
        else if(cur_type == CMARK_NODE_SOFTBREAK){
            if (ev_type == CMARK_EVENT_ENTER) {
                converted = converted + " ";
            }
        }
        else if(cur_type == CMARK_NODE_LINEBREAK){
            if (ev_type == CMARK_EVENT_ENTER) {
                converted = converted + "\n";
            }
        }
        else if(cur_type == CMARK_NODE_CODE){
            if (ev_type == CMARK_EVENT_ENTER) {
                converted = converted + "[code]";
                converted = converted + String::utf8(content);
                converted = converted + "[/code]";
                content_handled = 1;
            }
        }
        else if(cur_type == CMARK_NODE_HTML_INLINE){
        }
        else if(cur_type == CMARK_NODE_CUSTOM_INLINE){
        }
        else if(cur_type == CMARK_NODE_EMPH){
            if (ev_type == CMARK_EVENT_ENTER) {
                converted = converted + "[i]";
            }
            else if (ev_type == CMARK_EVENT_EXIT) {
                converted = converted + "[/i]";
            }
        }
        else if(cur_type == CMARK_NODE_STRONG){
            if (ev_type == CMARK_EVENT_ENTER) {
                converted = converted + "[b]";
            }
            else if (ev_type == CMARK_EVENT_EXIT) {
                converted = converted + "[/b]";
            }
        }
        else if(cur_type == CMARK_NODE_LINK){
            if (ev_type == CMARK_EVENT_ENTER) {
                converted = converted + "[url=";
                converted = converted + cmark_node_get_url(cur);
                converted = converted + "]";
                converted = converted + cmark_node_get_title(cur);
            }
            else if (ev_type == CMARK_EVENT_EXIT) {
                converted = converted + "[/url]";
            }
        }
        else if(cur_type == CMARK_NODE_IMAGE){
            if (ev_type == CMARK_EVENT_ENTER) {
                converted = converted + "[img]";
                converted = converted + cmark_node_get_url(cur);
            }
            else if (ev_type == CMARK_EVENT_EXIT) {
                converted = converted + "[/img]";
            }
        }
        else if(cur_type == CMARK_NODE_FOOTNOTE_REFERENCE){
        }
#ifndef _WINDOWS
        else if(cur_type == CMARK_NODE_STRIKETHROUGH){
            if (ev_type == CMARK_EVENT_ENTER) {
                converted = converted + "[s]";
            }
            else if (ev_type == CMARK_EVENT_EXIT) {
                converted = converted + "[/s]";
            }
        }
#endif
        else if(cur_type == CMARK_NODE_TABLE){
            if (ev_type == CMARK_EVENT_ENTER) {
                converted = converted + "[table=";
                converted = converted + String::num(cmark_gfm_extensions_get_table_columns(cur));
                converted = converted + "]";
            }
            else if (ev_type == CMARK_EVENT_EXIT) {
                converted = converted + "[/table]\n";
            }
        }
        else if(cur_type == CMARK_NODE_TABLE_ROW){
        }
        else if(cur_type == CMARK_NODE_TABLE_CELL){
            if (ev_type == CMARK_EVENT_ENTER) {
                converted = converted + "[cell]";
                converted = converted + String::utf8(content);
            }
            else if (ev_type == CMARK_EVENT_EXIT) {
                converted = converted + "[/cell]";
            }
        }
        if (content != NULL && !content_handled) {
            converted = converted + String::utf8(content);
        }
    }
    cmark_iter_free(iter);
    cmark_parser_free(parser);
    /* char* html = cmark_markdown_to_html(markdown, strlen(markdown), 0); */
    /* api->godot_string_new(&converted); */
    /* api->godot_string_parse_utf8(&converted, html); */
    // godot_string_operator_plus can concatenate two strings
    // godot_pool_string_array can concatenate many strings as far as I understand
    /* godot_string_array* converted_strings; */
    /* api->godot_pool_string_array_new(godot_string_array); */
    /* free(html); */
    cmark_node_free(document_root);

    return converted;
}
