#pragma once

#include <borealis/core/activity.hpp>
#include <borealis/core/bind.hpp>

#include "cpr/cpr.h"
#include <cpr/filesystem.h>
#include "view/auto_tab_frame.hpp"

class AutoTabFrame;
class RecyclingGrid;
class SVGImage;


class EntryInfo {
public:
    cpr::fs::path path;
    bool is_directory;
    bool is_first;
    uintmax_t file_size;
    std::filesystem::file_time_type last_write_time;
};

typedef brls::Event<EntryInfo&> PathSelectedEvent;

class LocalDisk : public AttachedView {
public:
    LocalDisk();

    ~LocalDisk();

    static View* create();

    void onCreate() override;
    void onUpdateDriver(const cpr::fs::path current);
private:
    BRLS_BIND(RecyclingGrid, recyclingGrid, "local/driver");
};