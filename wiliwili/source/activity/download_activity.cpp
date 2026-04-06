//
// Created by copilot on 2026/4/6.
//

#include <fmt/format.h>
#include <borealis/core/i18n.hpp>
#include <borealis/core/application.hpp>
#include <borealis/views/applet_frame.hpp>
#include <borealis/views/dialog.hpp>
#include <borealis/views/cells/cell_radio.hpp>

#include "activity/download_activity.hpp"
#include "utils/download_manager.hpp"
#include "utils/activity_helper.hpp"
#include "utils/image_helper.hpp"

using namespace brls::literals;

// ---------- DownloadCardCell ----------

DownloadCardCell::DownloadCardCell() {
    this->inflateFromXMLRes("xml/views/download_card.xml");
}

void DownloadCardCell::setData(const DownloadTask& task,
                               std::function<void(const std::string&)> btn1Action,
                               std::function<void(const std::string&)> btn2Action) {
    taskId = task.id;

    titleLabel->setText(task.title.empty() ? task.bvid : task.title);
    qualityLabel->setText(task.quality_desc);

    // Cover
    ImageHelper::clear(cover);
    if (!task.cover_url.empty()) {
        ImageHelper::with(cover)->load(task.cover_url + ImageHelper::h_ext);
    }

    // Progress text
    int64_t totalDl    = task.downloaded_bytes + task.audio_downloaded_bytes;
    int64_t totalBytes = task.total_bytes + task.audio_total_bytes;
    if (totalBytes > 0) {
        int pct = static_cast<int>(totalDl * 100 / totalBytes);
        progressLabel->setText(fmt::format("{}%  ({}/{} MB)", pct,
                                           totalDl / (1024 * 1024),
                                           totalBytes / (1024 * 1024)));
    } else if (totalDl > 0) {
        progressLabel->setText(fmt::format("{} MB", totalDl / (1024 * 1024)));
    } else {
        switch (task.status) {
            case DownloadTaskStatus::PENDING:
                progressLabel->setText("wiliwili/player/download/queued"_i18n);
                break;
            case DownloadTaskStatus::PAUSED:
                progressLabel->setText("...");
                break;
            case DownloadTaskStatus::FAILED:
                progressLabel->setText("wiliwili/player/download/failed"_i18n);
                break;
            case DownloadTaskStatus::COMPLETED:
                progressLabel->setText("wiliwili/player/download/completed"_i18n);
                break;
            default:
                progressLabel->setText("");
                break;
        }
    }

    // Buttons
    std::string id = task.id;
    if (task.status == DownloadTaskStatus::COMPLETED) {
        btn1->title->setText("wiliwili/download/manager/play"_i18n);
        btn1->registerClickAction([id, btn1Action](...) -> bool {
            if (btn1Action) btn1Action(id);
            return true;
        });
        btn2->title->setText("wiliwili/download/manager/delete"_i18n);
        btn2->registerClickAction([id, btn2Action](...) -> bool {
            if (btn2Action) btn2Action(id);
            return true;
        });
    } else if (task.status == DownloadTaskStatus::DOWNLOADING) {
        btn1->title->setText("wiliwili/download/manager/pause"_i18n);
        btn1->registerClickAction([id, btn1Action](...) -> bool {
            if (btn1Action) btn1Action(id);
            return true;
        });
        btn2->title->setText("wiliwili/download/manager/cancel"_i18n);
        btn2->registerClickAction([id, btn2Action](...) -> bool {
            if (btn2Action) btn2Action(id);
            return true;
        });
    } else if (task.status == DownloadTaskStatus::PAUSED) {
        btn1->title->setText("wiliwili/download/manager/resume"_i18n);
        btn1->registerClickAction([id, btn1Action](...) -> bool {
            if (btn1Action) btn1Action(id);
            return true;
        });
        btn2->title->setText("wiliwili/download/manager/cancel"_i18n);
        btn2->registerClickAction([id, btn2Action](...) -> bool {
            if (btn2Action) btn2Action(id);
            return true;
        });
    } else {
        // PENDING / FAILED / CANCELLED
        btn1->title->setText("wiliwili/download/manager/cancel"_i18n);
        btn1->registerClickAction([id, btn1Action](...) -> bool {
            if (btn1Action) btn1Action(id);
            return true;
        });
        btn2->title->setText("wiliwili/download/manager/delete"_i18n);
        btn2->registerClickAction([id, btn2Action](...) -> bool {
            if (btn2Action) btn2Action(id);
            return true;
        });
    }
}

// ---------- Data sources ----------

class ActiveDownloadDataSource : public RecyclingGridDataSource {
public:
    explicit ActiveDownloadDataSource(std::function<void()> refreshCb) : refreshCallback(std::move(refreshCb)) {}

    RecyclingGridItem* cellForRow(RecyclingGrid* recycler, size_t index) override {
        auto* cell = (DownloadCardCell*)recycler->dequeueReusableCell("Cell");
        auto& task = data[index];
        std::string id = task.id;

        auto btn1Action = [this, id](const std::string&) {
            auto* t = DownloadManager::instance().getTask(id);
            if (!t) return;
            if (t->status == DownloadTaskStatus::DOWNLOADING) {
                DownloadManager::instance().pauseTask(id);
            } else if (t->status == DownloadTaskStatus::PAUSED ||
                       t->status == DownloadTaskStatus::PENDING) {
                DownloadManager::instance().resumeTask(id);
            } else {
                DownloadManager::instance().cancelTask(id);
            }
            if (refreshCallback) refreshCallback();
        };

        auto btn2Action = [this, id](const std::string&) {
            auto dialog = new brls::Dialog("wiliwili/download/manager/confirm_cancel"_i18n);
            dialog->addButton("hints/cancel"_i18n, []() {});
            dialog->addButton("hints/ok"_i18n, [this, id]() {
                DownloadManager::instance().cancelTask(id);
                if (refreshCallback) refreshCallback();
            });
            dialog->open();
        };

        cell->setData(task, btn1Action, btn2Action);
        return cell;
    }

    size_t getItemCount() override { return data.size(); }
    void clearData() override { data.clear(); }

    std::vector<DownloadTask> data;

private:
    std::function<void()> refreshCallback;
};

class CompletedDownloadDataSource : public RecyclingGridDataSource {
public:
    explicit CompletedDownloadDataSource(std::function<void()> refreshCb) : refreshCallback(std::move(refreshCb)) {}

    RecyclingGridItem* cellForRow(RecyclingGrid* recycler, size_t index) override {
        auto* cell = (DownloadCardCell*)recycler->dequeueReusableCell("Cell");
        auto& task = data[index];
        std::string id = task.id;

        auto playAction = [id](const std::string&) {
            Intent::openLocalVideo(id);
        };

        auto deleteAction = [this, id](const std::string&) {
            auto dialog = new brls::Dialog("wiliwili/download/manager/confirm_delete"_i18n);
            dialog->addButton("hints/cancel"_i18n, []() {});
            dialog->addButton("hints/ok"_i18n, [this, id]() {
                DownloadManager::instance().deleteTask(id);
                if (refreshCallback) refreshCallback();
            });
            dialog->open();
        };

        cell->setData(task, playAction, deleteAction);
        return cell;
    }

    size_t getItemCount() override { return data.size(); }
    void clearData() override { data.clear(); }

    std::vector<DownloadTask> data;

private:
    std::function<void()> refreshCallback;
};

// ---------- DownloadActivity ----------

DownloadActivity::DownloadActivity() = default;
DownloadActivity::~DownloadActivity() {
    DownloadManager::instance().taskProgressEvent.unsubscribe(progressSub);
    DownloadManager::instance().taskStatusChangedEvent.unsubscribe(statusSub);
}

void DownloadActivity::onContentAvailable() {
    activeList->registerCell("Cell", []() { return DownloadCardCell::create(); });
    completedList->registerCell("Cell", []() { return DownloadCardCell::create(); });

    refreshLists();

    progressSub = DownloadManager::instance().taskProgressEvent.subscribe([this](const std::string&) {
        refreshLists();
    });
    statusSub = DownloadManager::instance().taskStatusChangedEvent.subscribe([this](const std::string&) {
        refreshLists();
    });
}

void DownloadActivity::refreshLists() {
    auto refreshCb = [this]() { this->refreshLists(); };

    auto* activeSrc    = new ActiveDownloadDataSource(refreshCb);
    auto* completedSrc = new CompletedDownloadDataSource(refreshCb);

    for (auto& t : DownloadManager::instance().getTasks()) {
        if (t.status == DownloadTaskStatus::COMPLETED) {
            completedSrc->data.push_back(t);
        } else if (t.status != DownloadTaskStatus::CANCELLED) {
            activeSrc->data.push_back(t);
        }
    }

    activeList->setDataSource(activeSrc);
    completedList->setDataSource(completedSrc);
}
