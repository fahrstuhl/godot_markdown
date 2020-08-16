#include <gdnative_api_struct.gen.h>
#include <string.h>
#include "cmark-gfm.h"

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

    cmark_node * document_root = cmark_parse_document(markdown, strlen(markdown), 0);
    cmark_iter *iter = cmark_iter_new(document_root);
    cmark_event_type ev_type;
    while ((ev_type = cmark_iter_next(iter)) != CMARK_EVENT_DONE) {
        cmark_node *cur = cmark_iter_get_node(iter);
        cmark_node_type cur_type = cmark_node_get_type(cur);
        const char* content = cmark_node_get_literal(cur);
        if (content != NULL) {
            godot_string content_gd;
            api->godot_string_new(&content_gd);
            api->godot_string_parse_utf8(&content_gd, content);
            converted = api->godot_string_operator_plus(&converted, &content_gd);
            api->godot_string_destroy(&content_gd);
        }
        if (ev_type == CMARK_EVENT_ENTER) {

        }
        else if (ev_type == CMARK_EVENT_EXIT) {

        }
    }
    cmark_iter_free(iter);
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
