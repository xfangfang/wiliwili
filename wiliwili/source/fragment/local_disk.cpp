//
// Created by Anonymous on 2024/3/31.
//

#include "fragment/local_disk.hpp"

#include "view/recycling_grid.hpp"
#include "view/svg_image.hpp"

#include "utils/activity_helper.hpp"

typedef std::vector<EntryInfo> LocalDiskResult;

const std::vector<std::string> SupportedExtensions = {
    ".8svx",
    ".aac",
    ".ac3",
    ".aif",
    ".asf",
    ".avi",
    ".dv",
    ".flv",
    ".m2ts",
    ".m2v",
    ".m4a",
    ".mkv",
    ".mov",
    ".mp3",
    ".mp4",
    ".mpeg",
    ".mpg",
    ".mts",
    ".ogg",
    ".rmvb",
    ".swf",
    ".ts",
    ".vob",
    ".wav",
    ".wma",
    ".wmv",
    ".flac",
    ".m3u",
    ".m3u8",
    ".webm",
    ".jpg",
    ".gif",
    ".png",
    ".iso",
    ".zip",
    ".tar",
    ".rar",
    ".gz",
};

std::string format_time(const std::filesystem::file_time_type time_ftt, const std::string& fmt) {
    auto sec = std::chrono::duration_cast<std::chrono::seconds>(time_ftt.time_since_epoch());

    std::time_t t = sec.count();
    const tm* lt = std::localtime(&t);

    std::stringstream ss;
    ss << std::put_time(lt, fmt.c_str());

    return ss.str();
}

std::string readable_fs(double size) {
    int i = 0;
    const char* units[] = {"B", "kB", "MB", "GB", "TB", "PB", "EB", "ZB", "YB"};
    while (size > 1024) {
        size /= 1024;
        i++;
    }
    return fmt::format("{:.{}f} {}", size, 2, units[i]);
}

class LocalDiskView : public RecyclingGridItem {
public:
    explicit LocalDiskView(const std::string& xml) { this->inflateFromXMLRes(xml); }

    void setInfo(const EntryInfo& info) {
        this->label_time->setText("");
        this->label_size->setText("");

        if (info.is_first) {
            this->label_path->setText("..");
            this->icon->setImageFromSVGRes("svg/ico-local-folder.svg");
            return;
        }

        this->label_path->setText(info.path.filename().string());
        this->label_time->setText(format_time(info.last_write_time, "%Y-%m-%d %H:%M:%S"));
        if (info.is_directory) {
            this->icon->setImageFromSVGRes("svg/ico-local-folder.svg");
        } else {
            this->icon->setImageFromSVGRes("svg/ico-local-file.svg");
            this->label_size->setText(readable_fs(info.file_size));
        }
    }

    static RecyclingGridItem* create(const std::string& xml = "xml/views/local_disk_item.xml") {
        return new LocalDiskView(xml);
    }
private:
    BRLS_BIND(brls::Label, label_path, "local/disk/item/path");
    BRLS_BIND(brls::Label, label_time, "local/disk/item/time");
    BRLS_BIND(brls::Label, label_size, "local/disk/item/size");
    BRLS_BIND(SVGImage, icon, "icon");
};


class DataSourceLocalDisk : public RecyclingGridDataSource {
public:
    explicit DataSourceLocalDisk(LocalDiskResult root) : list(std::move(root)) {}
    RecyclingGridItem* cellForRow(RecyclingGrid* recycler, size_t index) override {
        LocalDiskView* item = (LocalDiskView*)recycler->dequeueReusableCell("Cell");

        item->setInfo(this->list[index]);

        return item;
    }

    size_t getItemCount() override { return list.size(); }

    void onItemSelected(RecyclingGrid* recycler, size_t index) override {
        pathSelectedEvent.fire(list[index]);
    }

    void appendData(const EntryInfo data) {
        this->list.push_back(data);
    }

    // mode 0 by filesize
    // mode 1 by filename
    void sort(int mode, bool desc = true) {
        std::stable_sort(std::next(this->list.begin(), 1), this->list.end(),
                         [mode, desc](const EntryInfo& a, const EntryInfo& b) {
                             switch(mode) {
                                 case 1:
                                     if (desc) {
                                         return a.path.filename() < b.path.filename();
                                     } else {
                                         return a.path.filename() > b.path.filename();
                                     }
                                 default:
                                     if (desc) {
                                         return a.file_size < b.file_size;
                                     } else {
                                         return a.file_size > b.file_size;
                                     }
                             }
                         });
    }

    PathSelectedEvent * getSelectedEvent() { return &this->pathSelectedEvent; }

    void clearData() override { this->list.clear(); }

private:
    LocalDiskResult list;
    PathSelectedEvent pathSelectedEvent;
};

LocalDisk::LocalDisk() {
    this->inflateFromXMLRes("xml/fragment/local_disk.xml");
    brls::Logger::debug("Fragment LocalDisk: create");
}

LocalDisk::~LocalDisk() {
    brls::Logger::debug("Fragment LocalDiskActivity: delete");
    this->recyclingGrid->clearData();
}

brls::View* LocalDisk::create() { return new LocalDisk(); }

void LocalDisk::onUpdateDriver(const cpr::fs::path current) {
    auto datasource = (DataSourceLocalDisk*)this->recyclingGrid->getDataSource();
    datasource->clearData();

    datasource->appendData(EntryInfo{
        .path = current.parent_path(),
        .is_directory = true,
        .is_first = true,
    });
    for (auto const& dir_entry : cpr::fs::directory_iterator{current}) {
        if (dir_entry.exists() && dir_entry.path().filename().c_str()[0] != '.') {
            auto info = EntryInfo{
                .path            = dir_entry.path(),
                .is_directory    = dir_entry.is_directory(),
                .last_write_time = dir_entry.last_write_time(),
            };
            if (!info.is_directory) {
                info.file_size = dir_entry.file_size();
            }
            datasource->appendData(info);
        }
    }

    datasource->sort(0, false);

    this->recyclingGrid->reloadData();
}

void LocalDisk::onCreate() {
    this->recyclingGrid->registerCell("Cell", []() {return LocalDiskView::create();});

    auto datasource = new DataSourceLocalDisk(LocalDiskResult());

    datasource->getSelectedEvent()->subscribe([this](const EntryInfo &info) {
        if (info.path.empty()) return;

        brls::Logger::info("Select Path: {}", info.path.string());

        if (info.is_directory) {
            this->onUpdateDriver(info.path);
            return;
        }

        for (auto const& ext: SupportedExtensions) {
            if (info.path.extension() == ext) {
                Intent::openLocal(info.path);
                break;
            }
        }
    });

    this->recyclingGrid->setDataSource(datasource);
    this->onUpdateDriver(cpr::fs::current_path());
}

