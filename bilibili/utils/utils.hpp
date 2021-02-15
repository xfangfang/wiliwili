#pragma once

#include <fstream>
#include <string>
#include <map>
#include <vector>
#include <nlohmann/json.hpp>


typedef std::map<std::string, std::string> Cookie;

class ProgramConfig {
    public:
        ProgramConfig(){
            for(auto item: COOKIE_LIST){
                this->cookie[item] = "";
            }
        }
        void setCookie(std::vector<std::string> cookies){
            if (cookies.size() == COOKIE_LIST.size()){
                for(size_t i =0 ; i < COOKIE_LIST.size(); i++){
                    this->cookie[COOKIE_LIST[i]] = cookies[i];
                }
            }
        }
        Cookie getCookie(){
            return this->cookie;
        }
        Cookie cookie;
        const std::vector<std::string> COOKIE_LIST  = {"DedeUserID", "DedeUserID_ckMd5", "SESSDATA", "bili_jct"};
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ProgramConfig, cookie);

class Utils {
    static ProgramConfig programConfig;
    public:
        static ProgramConfig readProgramConf(){
            const std::string path = Utils::_getConfigPath()+"wiliwili_program.conf";
            ProgramConfig config;
            nlohmann::json content;
            std::ifstream readFile(path);
            if(!readFile) return config;
            try{
                readFile >> content;
                readFile.close();
                return content.get<ProgramConfig>();
            }
            catch(const std::exception& e){
                return config;
            }  
            return config;
        }

        static void saveProgramConf(ProgramConfig conf){
            const std::string path = Utils::_getConfigPath()+"wiliwili_program.conf";
            nlohmann::json content(conf);
            std::ofstream writeFile(path);
            if(!writeFile) return;
            writeFile << content.dump();
            writeFile.close();
        }

        static void readThemeConf(){
            std::string path = Utils::_getConfigPath()+"wiliwili_theme.conf";
        }

        static std::string _getConfigPath(){
            #ifdef __SWITCH__
                return "/";
            #else
                return "./";
            #endif
        }
};