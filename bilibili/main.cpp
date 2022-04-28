
/*
    WiliWili, a Custom BiliBili Client
    Copyright (C) 2021  xfangfang

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "application.hpp"
#include "utils/utils.hpp"

// global config
ProgramConfig Utils::programConfig = Utils::readProgramConf();

// run_on_ui callbacks
std::queue<std::function<void()>> RunOnUIThread::callbacks;


int main(int argc, char* argv[])
{

    if (Application::init()) return EXIT_FAILURE;
    
    Application::buildPages();

    Application::mainloop();
    
    Application::clean();

    return EXIT_SUCCESS;
}