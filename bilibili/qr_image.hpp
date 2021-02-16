#pragma once

#include <borealis/image.hpp>
#include "QrCode.hpp"

using qrcodegen::QrCode;
using qrcodegen::QrSegment;


class QrImage : public brls::Image {

    public:
        QrImage(std::string content = "")
        {
            brls::Logger::error("string {}",content.compare(""));
            if(content.compare("") == 0) {
                brls::Logger::error("just draw border");
                // only draw border
                int size = 20;
                for(int i = 0; i < size; i++){
                    std::vector<int> tmp;
                    for(int j = 0; j < size; j++){
                        if(i == 0 || i == size-1 || j == 0 || j == size-1) tmp.push_back(1);
                        else tmp.push_back(0);
                    }
                    this->qrcode.push_back(tmp);
                }
            } else {
                this->setImage(content);
            }
        }

        void setImage(std::string content)
        {
            this->qrcode.clear();
            const QrCode qr = QrCode::encodeText(content.c_str(), QrCode::Ecc::LOW);
            int border = 1;
            for (int y = -border; y < qr.getSize() + border; y++) {
                std::vector<int> tmp;
            	for (int x = -border; x < qr.getSize() + border; x++) {
                    if (qr.getModule(x, y)){
                        tmp.push_back(1);
                    } else {
                        tmp.push_back(0);
                    }                    
            	}
                this->qrcode.push_back(tmp);
            }
        }

        void draw(NVGcontext* vg, int x, int y, unsigned width, unsigned height, brls::Style* style, brls::FrameContext* ctx)
        {
            int unit_num = this->qrcode.size();
            float unit_length = ( height < width ? (float)height : (float)width ) / unit_num;
            for(size_t i = 0; i < this->qrcode.size(); i++){
                std::vector<int> & line = this->qrcode[i];
                for(size_t j = 0; j < line.size(); j++){
                    nvgBeginPath(vg);
                    nvgRect(vg, x + j * unit_length, y + i * unit_length, unit_length, unit_length);
                    nvgFillColor(vg, line[j] ? NVGcolor{0,0,0,255} : NVGcolor{255,255,255,255});
                    nvgFill(vg);
                }
            }
        }

    private:
        std::string content;
        std::vector<std::vector<int>> qrcode;
};