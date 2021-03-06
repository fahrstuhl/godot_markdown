#include <gdnative_api_struct.gen.h>
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

const godot_gdnative_core_api_struct *api = NULL;
const godot_gdnative_ext_nativescript_api_struct *nativescript_api = NULL;

void *cmark_constructor(godot_object *p_instance, void *p_method_data);
void cmark_destructor(godot_object *p_instance, void *p_method_data, void *p_user_data);
godot_variant cmark_convert_markdown(godot_object *p_instance, void *p_method_data,
        void *p_user_data, int p_num_args, godot_variant **p_args);

typedef struct user_data_struct {
    char data[256];
} user_data_struct;

void GDN_EXPORT godot_gdnative_init(godot_gdnative_init_options *p_options) {
    api = p_options->api_struct;

    // Now find our extensions.
    for (int i = 0; i < api->num_extensions; i++) {
        switch (api->extensions[i]->type) {
            case GDNATIVE_EXT_NATIVESCRIPT: {
                                                nativescript_api = (godot_gdnative_ext_nativescript_api_struct *)api->extensions[i];
                                            }; break;
            default: break;
        }
    }
}

void GDN_EXPORT godot_gdnative_terminate(godot_gdnative_terminate_options *p_options) {
    api = NULL;
    nativescript_api = NULL;
}

void GDN_EXPORT godot_nativescript_init(void *p_handle) {
    godot_instance_create_func create = { NULL, NULL, NULL };
    create.create_func = &cmark_constructor;

    godot_instance_destroy_func destroy = { NULL, NULL, NULL };
    destroy.destroy_func = &cmark_destructor;

    nativescript_api->godot_nativescript_register_class(p_handle, "CMark", "Reference",
            create, destroy);

    godot_instance_method convert_markdown = { NULL, NULL, NULL };
    convert_markdown.method = &cmark_convert_markdown;


    godot_method_attributes attributes = { GODOT_METHOD_RPC_MODE_DISABLED };

    nativescript_api->godot_nativescript_register_method(p_handle, "CMark", "convert_markdown",
            attributes, convert_markdown);
}

void *cmark_constructor(godot_object *p_instance, void *p_method_data) {
    user_data_struct *user_data = api->godot_alloc(sizeof(user_data_struct));
    strcpy(user_data->data, "[test](what.re)");

    return user_data;
}

void cmark_destructor(godot_object *p_instance, void *p_method_data, void *p_user_data) {
    api->godot_free(p_user_data);
}

void append_to_godot_string(godot_string *gd_dest, const char* src) {
    godot_string src_gd;
    api->godot_string_new(&src_gd);
    api->godot_string_parse_utf8(&src_gd, src);
    *gd_dest = api->godot_string_operator_plus(gd_dest, &src_gd);
    api->godot_string_destroy(&src_gd);
}

godot_variant cmark_convert_markdown(godot_object *p_instance, void *p_method_data,
        void *p_user_data, int p_num_args, godot_variant **p_args) {
    godot_string data;
    data = api->godot_variant_as_string(p_args[0]);

    godot_char_string parsed;
    godot_string converted;
    api->godot_string_new(&converted);
    godot_variant ret;
    parsed = api->godot_string_utf8(&data);
    const char* markdown = api->godot_char_string_get_data(&parsed);

    cmark_gfm_core_extensions_ensure_registered();
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
    cmark_parser_feed(parser, markdown, strlen(markdown));
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
                append_to_godot_string(&converted, "[indent]");
            }
            else if (ev_type == CMARK_EVENT_EXIT) {
                append_to_godot_string(&converted, "[/indent]");
            }
        }
        else if(cur_type == CMARK_NODE_LIST){
            if (ev_type == CMARK_EVENT_ENTER) {
                /* append_to_godot_string(&converted, "<li>"); */
                list_level += 1;
            }
            else if (ev_type == CMARK_EVENT_EXIT) {
                /* append_to_godot_string(&converted, "</li>"); */
                list_level -= 1;
            }
        }
        else if(cur_type == CMARK_NODE_ITEM){
            if (ev_type == CMARK_EVENT_ENTER) {
                if(list_level >= 1) {
                    append_to_godot_string(&converted, "[indent]");
                }
                parent = cmark_node_parent(cur);
                char* tasklist_state = cmark_gfm_extensions_get_tasklist_state(cur);
                bool is_tasklist = strcmp(cmark_node_get_type_string(cur), "tasklist") == 0;
                if (parent != NULL
                        && cmark_node_get_type(parent) == CMARK_NODE_LIST
                        && cmark_node_get_list_type(parent) == CMARK_ORDERED_LIST) {
                    int list_start = cmark_node_get_list_start(parent);
                    godot_string num_gd = api->godot_string_num_int64(list_start, 10);
                    converted = api->godot_string_operator_plus(&converted, &num_gd);
                    append_to_godot_string(&converted, ". ");
                    api->godot_string_destroy(&num_gd);
                    cmark_node_set_list_start(parent, list_start + 1);
                }
                else if (!is_tasklist) {
                    append_to_godot_string(&converted, "• ");
                }
                else if (strcmp(tasklist_state, "checked") == 0) {
                    append_to_godot_string(&converted, "🔲 ");
                }
                else if (strcmp(tasklist_state, "unchecked") == 0) {
                    append_to_godot_string(&converted, "☑ ");
                }
            }
            else if (ev_type == CMARK_EVENT_EXIT) {
                if(list_level >= 1) {
                    append_to_godot_string(&converted, "[/indent]");
                }
            }
        }
        else if(cur_type == CMARK_NODE_CODE_BLOCK){
            if (ev_type == CMARK_EVENT_ENTER) {
                append_to_godot_string(&converted, "[indent][code]");
                append_to_godot_string(&converted, content);
                append_to_godot_string(&converted, "[/code][/indent]");
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
                    append_to_godot_string(&converted, "  ");
                }
                /* append_to_godot_string(&converted, "<p>"); */
            }
            else if (ev_type == CMARK_EVENT_EXIT) {
                /* append_to_godot_string(&converted, "</p>"); */
                if (is_child_of_item) {
                    if (is_not_first_child_of_item) {
                        /* append_to_godot_string(&converted, "[/indent]"); */
                    }
                    if (!tight) {
                        append_to_godot_string(&converted, "\n\n");
                    }
                    else {
                        append_to_godot_string(&converted, "\n");
                    }
                } else {
                    append_to_godot_string(&converted, "\n\n");
                }
            }
        }
        else if(cur_type == CMARK_NODE_HEADING){
            if (ev_type == CMARK_EVENT_ENTER) {
                append_to_godot_string(&converted, "[");
            }
            else if (ev_type == CMARK_EVENT_EXIT) {
                append_to_godot_string(&converted, "[/");
            }
            append_to_godot_string(&converted, "heading_");
            int heading_level = cmark_node_get_heading_level(cur);
            godot_string num_gd = api->godot_string_num_int64(heading_level, 10);
            converted = api->godot_string_operator_plus(&converted, &num_gd);
            append_to_godot_string(&converted, "]");
            if (ev_type == CMARK_EVENT_EXIT) {
                append_to_godot_string(&converted, "\n");
            }
        }
        else if(cur_type == CMARK_NODE_THEMATIC_BREAK){
            append_to_godot_string(&converted, "---\n\n");
        }
        else if(cur_type == CMARK_NODE_FOOTNOTE_DEFINITION){
        }
        else if(cur_type == CMARK_NODE_TEXT){
        }
        else if(cur_type == CMARK_NODE_SOFTBREAK){
            if (ev_type == CMARK_EVENT_ENTER) {
                append_to_godot_string(&converted, " ");
            }
        }
        else if(cur_type == CMARK_NODE_LINEBREAK){
            if (ev_type == CMARK_EVENT_ENTER) {
                append_to_godot_string(&converted, "\n");
            }
        }
        else if(cur_type == CMARK_NODE_CODE){
            if (ev_type == CMARK_EVENT_ENTER) {
                append_to_godot_string(&converted, "[code]");
                append_to_godot_string(&converted, content);
                append_to_godot_string(&converted, "[/code]");
                content_handled = 1;
            }
        }
        else if(cur_type == CMARK_NODE_HTML_INLINE){
        }
        else if(cur_type == CMARK_NODE_CUSTOM_INLINE){
        }
        else if(cur_type == CMARK_NODE_EMPH){
            if (ev_type == CMARK_EVENT_ENTER) {
                append_to_godot_string(&converted, "[i]");
            }
            else if (ev_type == CMARK_EVENT_EXIT) {
                append_to_godot_string(&converted, "[/i]");
            }
        }
        else if(cur_type == CMARK_NODE_STRONG){
            if (ev_type == CMARK_EVENT_ENTER) {
                append_to_godot_string(&converted, "[b]");
            }
            else if (ev_type == CMARK_EVENT_EXIT) {
                append_to_godot_string(&converted, "[/b]");
            }
        }
        else if(cur_type == CMARK_NODE_LINK){
            if (ev_type == CMARK_EVENT_ENTER) {
                append_to_godot_string(&converted, "[url=");
                append_to_godot_string(&converted, cmark_node_get_url(cur));
                append_to_godot_string(&converted, "]");
                append_to_godot_string(&converted, cmark_node_get_title(cur));
            }
            else if (ev_type == CMARK_EVENT_EXIT) {
                append_to_godot_string(&converted, "[/url]");
            }
        }
        else if(cur_type == CMARK_NODE_IMAGE){
            if (ev_type == CMARK_EVENT_ENTER) {
                append_to_godot_string(&converted, "[img]");
                append_to_godot_string(&converted, cmark_node_get_url(cur));
            }
            else if (ev_type == CMARK_EVENT_EXIT) {
                append_to_godot_string(&converted, "[/img]");
            }
        }
        else if(cur_type == CMARK_NODE_FOOTNOTE_REFERENCE){
        }
#ifndef _WINDOWS
        else if(cur_type == CMARK_NODE_STRIKETHROUGH){
            if (ev_type == CMARK_EVENT_ENTER) {
                append_to_godot_string(&converted, "[s]");
            }
            else if (ev_type == CMARK_EVENT_EXIT) {
                append_to_godot_string(&converted, "[/s]");
            }
        }
#endif
        else if(cur_type == CMARK_NODE_TABLE){
        }
        else if(cur_type == CMARK_NODE_TABLE_ROW){
        }
        else if(cur_type == CMARK_NODE_TABLE_CELL){
        }
        if (content != NULL && !content_handled) {
            append_to_godot_string(&converted, content);
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
    api->godot_variant_new_string(&ret, &converted);

    api->godot_string_destroy(&data);
    api->godot_char_string_destroy(&parsed);
    /* free(html); */
    cmark_node_free(document_root);
    api->godot_string_destroy(&converted);

    return ret;
}
