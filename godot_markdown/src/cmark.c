#include <godot/gdnative_interface.h>
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

const char parent_name[] = "Object";
const char class_name[] = "CMark";
const char method_name[] = "convert_markdown";

const GDNativeInterface *gdn_interface = NULL;
GDNativeExtensionClassLibraryPtr library = NULL;

/* void *cmark_constructor(godot_object *p_instance, void *p_method_data); */
/* void cmark_destructor(godot_object *p_instance, void *p_method_data, void *p_user_data); */
/* godot_variant cmark_convert_markdown(godot_object *p_instance, void *p_method_data, */
/*         void *p_user_data, int p_num_args, godot_variant **p_args); */

typedef struct user_data_struct {
    char data[256];
} user_data_struct;

void initialize_level(void *userdata, GDNativeInitializationLevel p_level) {
}

void deinitialize_level(void *userdata, GDNativeInitializationLevel p_level) {
	/* void (*classdb_unregister_extension_class)(const GDNativeExtensionClassLibraryPtr p_library, const char *p_class_name); /1* Unregistering a parent class before a class that inherits it will result in failure. Inheritors must be unregistered first. *1/ */
}

static GDNativeObjectPtr create_md(void *data) {
    gdn_interface->print_warning("creating CMark", __func__, __FILE__, __LINE__);
    GDNativeObjectPtr class_ptr = gdn_interface->classdb_construct_object("Object");
    GDExtensionClassInstancePtr instance = gdn_interface->mem_alloc(4);
    gdn_interface->object_set_instance(class_ptr, "CMark", instance);
    return class_ptr;
}

static void free_md(void *data, GDExtensionClassInstancePtr ptr) {
    gdn_interface->print_warning("freeing CMark", __func__, __FILE__, __LINE__);
    if (ptr) {
    	gdn_interface->mem_free(ptr);
        gdn_interface = NULL;
        library = NULL;
    }
}

void append_to_godot_string(GDNativeStringPtr gd_a, const char* str_b) {
    GDNativeBool valid = 0;
    GDNativeStringPtr gd_b;
    gdn_interface->string_new_with_utf8_chars(gd_b, str_b);
    gdn_interface->variant_evaluate(GDNATIVE_VARIANT_OP_ADD, gd_a, gd_b, gd_a, &valid);
    gdn_interface->object_destroy(gd_a);
}

GDNativeStringPtr cmark_convert_markdown() {
    gdn_interface->print_warning("converting markdown", __func__, __FILE__, __LINE__);
    GDNativeStringPtr gd_test = gdn_interface->classdb_construct_object("String");
    gdn_interface->string_new_with_utf8_chars(gd_test, "testtesttesttest");
    gdn_interface->print_warning("finished converting markdown", __func__, __FILE__, __LINE__);
    return gd_test;
}

void cmark_call(void *p_method_userdata, GDExtensionClassInstancePtr p_instance, const GDNativeVariantPtr *p_args, const GDNativeInt p_argument_count, GDNativeVariantPtr r_return, GDNativeCallError *r_error) {
    gdn_interface->print_warning("call", __func__, __FILE__, __LINE__);
    r_return = cmark_convert_markdown();
};

void cmark_ptrcall(void *p_method_userdata, GDExtensionClassInstancePtr p_instance, const GDNativeTypePtr *p_args, GDNativeTypePtr r_return) {
    gdn_interface->print_warning("ptrcall", __func__, __FILE__, __LINE__);
    r_return = cmark_convert_markdown();
};

GDNativeVariantType cmark_get_argument_type(void *p_method_userdata, int32_t p_argument) {
    gdn_interface->print_warning("getting argument type", __func__, __FILE__, __LINE__);
    if (p_argument == 0 || p_argument == -1) {
        return GDNATIVE_VARIANT_TYPE_STRING;
    }
    else {
        return GDNATIVE_VARIANT_TYPE_NIL;
    }
}

void cmark_get_argument_info(void *p_method_userdata, int32_t p_argument, GDNativePropertyInfo *r_info) {
    gdn_interface->print_warning("getting argument info", __func__, __FILE__, __LINE__);
    if (p_argument == 0 || p_argument == -1) {
    	r_info->type = GDNATIVE_VARIANT_TYPE_STRING;
    	r_info->name = "markdown_text";
    	r_info->class_name = "String";
    	r_info->usage = 0;
    }
}

GDNativeExtensionClassMethodArgumentMetadata cmark_get_argument_metadata(void *p_method_userdata, int32_t p_argument) {
    gdn_interface->print_warning("getting argument metadata", __func__, __FILE__, __LINE__);
    return GDNATIVE_EXTENSION_METHOD_ARGUMENT_METADATA_NONE;
}

GDNativeBool godot_cmark_init(const GDNativeInterface *p_interface, const GDNativeExtensionClassLibraryPtr p_library, GDNativeInitialization *r_initialization) {
    gdn_interface = p_interface;
    library = p_library;
    r_initialization->initialize = &initialize_level;
    r_initialization->deinitialize = &deinitialize_level;
    const GDNativeExtensionClassCreationInfo class_info = {
		NULL, // GDNativeExtensionClassSet set_func;
		NULL, // GDNativeExtensionClassGet get_func;
		NULL, // GDNativeExtensionClassGetPropertyList get_property_list_func;
		NULL, // GDNativeExtensionClassFreePropertyList free_property_list_func;
		NULL, // GDNativeExtensionClassNotification notification_func;
		NULL, // GDNativeExtensionClassToString to_string_func;
		NULL, // GDNativeExtensionClassReference reference_func;
		NULL, // GDNativeExtensionClassUnreference
		create_md, // GDNativeExtensionClassCreateInstance create_instance_func; /* this one is mandatory */
		free_md, // GDNativeExtensionClassFreeInstance free_instance_func; /* this one is mandatory */
		NULL, // GDNativeExtensionClassGetVirtual get_virtual_func;
		NULL, // void *class_userdata;
	};
    gdn_interface->classdb_register_extension_class(library, class_name, parent_name, &class_info);
	GDNativeExtensionClassMethodInfo method_info = {
		method_name, //const char *name;
		cmark_convert_markdown, //void *method_userdata;
		cmark_call, //GDNativeExtensionClassMethodCall call_func;
		cmark_ptrcall, //GDNativeExtensionClassMethodPtrCall ptrcall_func;
		GDNATIVE_EXTENSION_METHOD_FLAGS_DEFAULT, //uint32_t method_flags; /* GDNativeExtensionClassMethodFlags */
		1, //uint32_t argument_count;
		1, //GDNativeBool has_return_value;
		cmark_get_argument_type, //(GDNativeExtensionClassMethodGetArgumentType) get_argument_type_func;
		cmark_get_argument_info, //GDNativeExtensionClassMethodGetArgumentInfo get_argument_info_func; /* name and hint information for the argument can be omitted in release builds. Class name should always be present if it applies. */
		cmark_get_argument_metadata, //GDNativeExtensionClassMethodGetArgumentMetadata get_argument_metadata_func;
		0, //uint32_t default_argument_count;
		NULL, //GDNativeVariantPtr *default_arguments;
	};
	gdn_interface->classdb_register_extension_class_method(library, class_name, &method_info);
	/* void (*classdb_unregister_extension_class)(const GDNativeExtensionClassLibraryPtr p_library, const char *p_class_name); /1* Unregistering a parent class before a class that inherits it will result in failure. Inheritors must be unregistered first. *1/ */

    gdn_interface->print_warning("class registered", __func__, __FILE__, __LINE__);
    return 1;
}

/* godot_variant cmark_convert_markdown(godot_object *p_instance, void *p_method_data, */
/*         void *p_user_data, int p_num_args, godot_variant **p_args) { */
/*     godot_string data; */
/*     data = api->godot_variant_as_string(p_args[0]); */

/*     godot_char_string parsed; */
/*     godot_string converted; */
/*     api->godot_string_new(&converted); */
/*     godot_variant ret; */
/*     parsed = api->godot_string_utf8(&data); */
/*     const char* markdown = api->godot_char_string_get_data(&parsed); */

/*     cmark_gfm_core_extensions_ensure_registered(); */
/*     cmark_parser * parser = cmark_parser_new(0); */
/*     for (const char **it = extension_names; *it; ++it) { */
/*         const char *extension_name = *it; */
/*         cmark_syntax_extension *syntax_extension = cmark_find_syntax_extension(extension_name); */
/*         if (!syntax_extension) { */
/*             fprintf(stderr, "%s is not a valid syntax extension\n", extension_name); */
/*             abort(); */
/*         } */
/*         cmark_parser_attach_syntax_extension(parser, syntax_extension); */
/*     } */
/*     cmark_parser_feed(parser, markdown, strlen(markdown)); */
/*     cmark_node * document_root = cmark_parser_finish(parser); */
/*     cmark_iter *iter = cmark_iter_new(document_root); */
/*     cmark_event_type ev_type; */
/*     cmark_node *parent; */
/*     cmark_node *grandparent; */
/*     int list_level = 0; */
/*     while ((ev_type = cmark_iter_next(iter)) != CMARK_EVENT_DONE) { */
/*         int content_handled = 0; */
/*         cmark_node *cur = cmark_iter_get_node(iter); */
/*         cmark_node_type cur_type = cmark_node_get_type(cur); */
/*         const char* content = cmark_node_get_literal(cur); */
/*         if (cur_type == CMARK_NODE_DOCUMENT){ */
/*         } */
/*         else if(cur_type == CMARK_NODE_BLOCK_QUOTE){ */
/*             if (ev_type == CMARK_EVENT_ENTER) { */
/*                 append_to_godot_string(&converted, "[indent]"); */
/*             } */
/*             else if (ev_type == CMARK_EVENT_EXIT) { */
/*                 append_to_godot_string(&converted, "[/indent]"); */
/*             } */
/*         } */
/*         else if(cur_type == CMARK_NODE_LIST){ */
/*             if (ev_type == CMARK_EVENT_ENTER) { */
/*                 /1* append_to_godot_string(&converted, "<li>"); *1/ */
/*                 list_level += 1; */
/*             } */
/*             else if (ev_type == CMARK_EVENT_EXIT) { */
/*                 /1* append_to_godot_string(&converted, "</li>"); *1/ */
/*                 list_level -= 1; */
/*             } */
/*         } */
/*         else if(cur_type == CMARK_NODE_ITEM){ */
/*             if (ev_type == CMARK_EVENT_ENTER) { */
/*                 if(list_level >= 1) { */
/*                     append_to_godot_string(&converted, "[indent]"); */
/*                 } */
/*                 parent = cmark_node_parent(cur); */
/*                 char* tasklist_state = cmark_gfm_extensions_get_tasklist_state(cur); */
/*                 bool is_tasklist = strcmp(cmark_node_get_type_string(cur), "tasklist") == 0; */
/*                 if (parent != NULL */
/*                         && cmark_node_get_type(parent) == CMARK_NODE_LIST */
/*                         && cmark_node_get_list_type(parent) == CMARK_ORDERED_LIST) { */
/*                     int list_start = cmark_node_get_list_start(parent); */
/*                     godot_string num_gd = api->godot_string_num_int64(list_start, 10); */
/*                     converted = api->godot_string_operator_plus(&converted, &num_gd); */
/*                     append_to_godot_string(&converted, ". "); */
/*                     api->godot_string_destroy(&num_gd); */
/*                     cmark_node_set_list_start(parent, list_start + 1); */
/*                 } */
/*                 else if (!is_tasklist) { */
/*                     append_to_godot_string(&converted, "• "); */
/*                 } */
/*                 else if (strcmp(tasklist_state, "checked") == 0) { */
/*                     append_to_godot_string(&converted, "🔲 "); */
/*                 } */
/*                 else if (strcmp(tasklist_state, "unchecked") == 0) { */
/*                     append_to_godot_string(&converted, "☑ "); */
/*                 } */
/*             } */
/*             else if (ev_type == CMARK_EVENT_EXIT) { */
/*                 if(list_level >= 1) { */
/*                     append_to_godot_string(&converted, "[/indent]"); */
/*                 } */
/*             } */
/*         } */
/*         else if(cur_type == CMARK_NODE_CODE_BLOCK){ */
/*             if (ev_type == CMARK_EVENT_ENTER) { */
/*                 append_to_godot_string(&converted, "[indent][code]"); */
/*                 append_to_godot_string(&converted, content); */
/*                 append_to_godot_string(&converted, "[/code][/indent]"); */
/*                 content_handled = 1; */
/*             } */
/*         } */
/*         else if(cur_type == CMARK_NODE_HTML_BLOCK){ */
/*         } */
/*         else if(cur_type == CMARK_NODE_CUSTOM_BLOCK){ */
/*         } */
/*         else if(cur_type == CMARK_NODE_PARAGRAPH){ */
/*             parent = cmark_node_parent(cur); */
/*             grandparent = cmark_node_parent(parent); */
/*             int tight; */
/*             int is_child_of_item = parent != NULL && cmark_node_get_type(parent) == CMARK_NODE_ITEM; */
/*             int is_not_first_child_of_item; */
/*             if (is_child_of_item) { */
/*                 is_not_first_child_of_item = cur != cmark_node_first_child(parent); */
/*             } */
/*             if (grandparent != NULL && cmark_node_get_type(grandparent) == CMARK_NODE_LIST) { */
/*                 tight = cmark_node_get_list_tight(grandparent); */
/*             } else { */
/*                 tight = false; */
/*             } */
/*             if (ev_type == CMARK_EVENT_ENTER) { */
/*                 if (is_child_of_item && is_not_first_child_of_item) { */
/*                     append_to_godot_string(&converted, "  "); */
/*                 } */
/*                 /1* append_to_godot_string(&converted, "<p>"); *1/ */
/*             } */
/*             else if (ev_type == CMARK_EVENT_EXIT) { */
/*                 /1* append_to_godot_string(&converted, "</p>"); *1/ */
/*                 if (is_child_of_item) { */
/*                     if (is_not_first_child_of_item) { */
/*                         /1* append_to_godot_string(&converted, "[/indent]"); *1/ */
/*                     } */
/*                     if (!tight) { */
/*                         append_to_godot_string(&converted, "\n\n"); */
/*                     } */
/*                     else { */
/*                         append_to_godot_string(&converted, "\n"); */
/*                     } */
/*                 } else { */
/*                     append_to_godot_string(&converted, "\n\n"); */
/*                 } */
/*             } */
/*         } */
/*         else if(cur_type == CMARK_NODE_HEADING){ */
/*             if (ev_type == CMARK_EVENT_ENTER) { */
/*                 append_to_godot_string(&converted, "["); */
/*             } */
/*             else if (ev_type == CMARK_EVENT_EXIT) { */
/*                 append_to_godot_string(&converted, "[/"); */
/*             } */
/*             append_to_godot_string(&converted, "heading_"); */
/*             int heading_level = cmark_node_get_heading_level(cur); */
/*             godot_string num_gd = api->godot_string_num_int64(heading_level, 10); */
/*             converted = api->godot_string_operator_plus(&converted, &num_gd); */
/*             append_to_godot_string(&converted, "]"); */
/*             if (ev_type == CMARK_EVENT_EXIT) { */
/*                 append_to_godot_string(&converted, "\n"); */
/*             } */
/*         } */
/*         else if(cur_type == CMARK_NODE_THEMATIC_BREAK){ */
/*             append_to_godot_string(&converted, "---\n\n"); */
/*         } */
/*         else if(cur_type == CMARK_NODE_FOOTNOTE_DEFINITION){ */
/*         } */
/*         else if(cur_type == CMARK_NODE_TEXT){ */
/*         } */
/*         else if(cur_type == CMARK_NODE_SOFTBREAK){ */
/*             if (ev_type == CMARK_EVENT_ENTER) { */
/*                 append_to_godot_string(&converted, " "); */
/*             } */
/*         } */
/*         else if(cur_type == CMARK_NODE_LINEBREAK){ */
/*             if (ev_type == CMARK_EVENT_ENTER) { */
/*                 append_to_godot_string(&converted, "\n"); */
/*             } */
/*         } */
/*         else if(cur_type == CMARK_NODE_CODE){ */
/*             if (ev_type == CMARK_EVENT_ENTER) { */
/*                 append_to_godot_string(&converted, "[code]"); */
/*                 append_to_godot_string(&converted, content); */
/*                 append_to_godot_string(&converted, "[/code]"); */
/*                 content_handled = 1; */
/*             } */
/*         } */
/*         else if(cur_type == CMARK_NODE_HTML_INLINE){ */
/*         } */
/*         else if(cur_type == CMARK_NODE_CUSTOM_INLINE){ */
/*         } */
/*         else if(cur_type == CMARK_NODE_EMPH){ */
/*             if (ev_type == CMARK_EVENT_ENTER) { */
/*                 append_to_godot_string(&converted, "[i]"); */
/*             } */
/*             else if (ev_type == CMARK_EVENT_EXIT) { */
/*                 append_to_godot_string(&converted, "[/i]"); */
/*             } */
/*         } */
/*         else if(cur_type == CMARK_NODE_STRONG){ */
/*             if (ev_type == CMARK_EVENT_ENTER) { */
/*                 append_to_godot_string(&converted, "[b]"); */
/*             } */
/*             else if (ev_type == CMARK_EVENT_EXIT) { */
/*                 append_to_godot_string(&converted, "[/b]"); */
/*             } */
/*         } */
/*         else if(cur_type == CMARK_NODE_LINK){ */
/*             if (ev_type == CMARK_EVENT_ENTER) { */
/*                 append_to_godot_string(&converted, "[url="); */
/*                 append_to_godot_string(&converted, cmark_node_get_url(cur)); */
/*                 append_to_godot_string(&converted, "]"); */
/*                 append_to_godot_string(&converted, cmark_node_get_title(cur)); */
/*             } */
/*             else if (ev_type == CMARK_EVENT_EXIT) { */
/*                 append_to_godot_string(&converted, "[/url]"); */
/*             } */
/*         } */
/*         else if(cur_type == CMARK_NODE_IMAGE){ */
/*             if (ev_type == CMARK_EVENT_ENTER) { */
/*                 append_to_godot_string(&converted, "[img]"); */
/*                 append_to_godot_string(&converted, cmark_node_get_url(cur)); */
/*             } */
/*             else if (ev_type == CMARK_EVENT_EXIT) { */
/*                 append_to_godot_string(&converted, "[/img]"); */
/*             } */
/*         } */
/*         else if(cur_type == CMARK_NODE_FOOTNOTE_REFERENCE){ */
/*         } */
/* #ifndef _WINDOWS */
/*         else if(cur_type == CMARK_NODE_STRIKETHROUGH){ */
/*             if (ev_type == CMARK_EVENT_ENTER) { */
/*                 append_to_godot_string(&converted, "[s]"); */
/*             } */
/*             else if (ev_type == CMARK_EVENT_EXIT) { */
/*                 append_to_godot_string(&converted, "[/s]"); */
/*             } */
/*         } */
/* #endif */
/*         else if(cur_type == CMARK_NODE_TABLE){ */
/*         } */
/*         else if(cur_type == CMARK_NODE_TABLE_ROW){ */
/*         } */
/*         else if(cur_type == CMARK_NODE_TABLE_CELL){ */
/*         } */
/*         if (content != NULL && !content_handled) { */
/*             append_to_godot_string(&converted, content); */
/*         } */
/*     } */
/*     cmark_iter_free(iter); */
/*     cmark_parser_free(parser); */
/*     /1* char* html = cmark_markdown_to_html(markdown, strlen(markdown), 0); *1/ */
/*     /1* api->godot_string_new(&converted); *1/ */
/*     /1* api->godot_string_parse_utf8(&converted, html); *1/ */
/*     // godot_string_operator_plus can concatenate two strings */
/*     // godot_pool_string_array can concatenate many strings as far as I understand */
/*     /1* godot_string_array* converted_strings; *1/ */
/*     /1* api->godot_pool_string_array_new(godot_string_array); *1/ */
/*     api->godot_variant_new_string(&ret, &converted); */

/*     api->godot_string_destroy(&data); */
/*     api->godot_char_string_destroy(&parsed); */
/*     /1* free(html); *1/ */
/*     cmark_node_free(document_root); */
/*     api->godot_string_destroy(&converted); */

/*     return ret; */
/* } */
