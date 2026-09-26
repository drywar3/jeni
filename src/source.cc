#include "source.h"

#include <mini.c/fs.h>
#include <mini.cc/dtor.h>

SourceManager sourcemgr_init() {
    SourceManager sm{ .sources = SourceManager::Map(mini_default_allocator()) };
    return sm;
}

void sourcemgr_destroy(SourceManager *sm) {
    for (auto source : sm->sources) {
        mini_array_destroy(source.canon_path);
        mini_array_destroy(source.path);
        mini_array_destroy(source.content);
        mini_array_destroy(source.line_starts);
    }
}

SourceId sourcemgr_open_file(SourceManager *sm, mini::StringView file_path, Mini_Allocator allocator) {
    Mini_String canon = mini_string_init(allocator);

    Mini_String file_path_real  = file_path.to_string();

    mini_fs_canonicalize(file_path_real, &canon);

    if (sm->sources.contains(canon)) {
        return SourceId(*sm->sources.get_id(canon));
    }

    Mini_String content = mini_string_init(allocator);
    MINI_ASSERT(mini_fs_read_into(file_path_real, &content),);

    MINI_ARRAY(usize) line_starts = MINI_ARRAY_INIT(allocator, usize);
    mini_array_append(line_starts, 0);

    for (usize n = 0; n < mini_string_count(content); n++) {
        if (content[n] == '\n') {
            mini_array_append(line_starts, n + 1);
        }
    }

    return SourceId(sm->sources.insert(file_path, SourceFile {
                .path  = file_path_real,
                .canon_path = canon,
                .content    = content,
                .line_starts = line_starts,
    }));
}

const Mini_String &sourcemgr_get_content(SourceManager *sm, SourceId id) {
    return sm->sources.at_index((usize)id).content;
}

const SourceFile *sourcemgr_get_source(const SourceManager *sm, SourceId id) {
    return &sm->sources.at_index((usize)id);
}
