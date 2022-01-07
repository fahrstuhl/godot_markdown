#include "cmark.h"

#include <godot_cpp/core/class_db.hpp>

#include <godot_cpp/classes/global_constants.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

void Cmark::_bind_methods() {
	ClassDB::bind_method(D_METHOD("convert_markdown"), &Cmark::convert_markdown);
}

Cmark::Cmark() {
	UtilityFunctions::print("Constructor.");
}

Cmark::~Cmark() {
	UtilityFunctions::print("Destructor.");
}

String Cmark::convert_markdown(const String &markdown) {
	UtilityFunctions::print("Return something called.");
	return markdown;
}
