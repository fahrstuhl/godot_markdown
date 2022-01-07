#ifndef CMARK_H
#define CMARK_H

#ifdef WIN32
#include <windows.h>
#endif

#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/classes/global_constants.hpp>

#include <godot_cpp/core/binder_common.hpp>

using namespace godot;

class Cmark : public Object {
	GDCLASS(Cmark, Object);

protected:
	static void _bind_methods();

public:
	Cmark();
	~Cmark();

	// Functions.
	void simple_func();
	String convert_markdown(const String &markdown);
};

#endif
