#pragma once

#include <mini.cc/string_view.h>

#include "misc/densemap.h"
#include "misc/misc.h"

enum struct SourceId : usize {};

struct SourceFile {
    Mini_String   path;
    Mini_String   canon_path;
    Mini_String   content;
    MINI_ARRAY(usize) line_starts;
};

struct SourceManager {
    using Map = DenseMap<mini::StringView, SourceFile>;
    Map sources;
};

SourceManager sourcemgr_init();
void          sourcemgr_destroy(SourceManager *sm);

SourceId sourcemgr_open_file(SourceManager *sm, mini::StringView file_path, Mini_Allocator allocator);

const Mini_String &sourcemgr_get_content(SourceManager *sm, SourceId id);
const SourceFile *sourcemgr_get_source(const SourceManager *sm, SourceId);
