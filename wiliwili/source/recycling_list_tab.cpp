/*
    Copyright 2020-2021 natinusala

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

#include "recycling_list_tab.hpp"
#include "pokemon_view.hpp"

std::vector<Pokemon> pokemons;

RecyclerCell::RecyclerCell()
{
    this->inflateFromXMLRes("xml/cells/cell.xml");
}

RecyclerCell* RecyclerCell::create()
{
    return new RecyclerCell();
}

// DATA SOURCE

int DataSource::numberOfSections(brls::RecyclerFrame* recycler)
{
    return 2;
}

int DataSource::numberOfRows(brls::RecyclerFrame* recycler, int section)
{
    return pokemons.size();
}
    
std::string DataSource::titleForHeader(brls::RecyclerFrame* recycler, int section) 
{
    if (section == 0)
        return "";
    return "Section #" + std::to_string(section+1);
}

brls::RecyclerCell* DataSource::cellForRow(brls::RecyclerFrame* recycler, brls::IndexPath indexPath)
{
    RecyclerCell* item = (RecyclerCell*)recycler->dequeueReusableCell("Cell");
    item->label->setText(pokemons[indexPath.row].name);
    item->image->setImageFromRes("img/pokemon/thumbnails/" + pokemons[indexPath.row].id + ".png");
    return item;
}

void DataSource::didSelectRowAt(brls::RecyclerFrame* recycler, brls::IndexPath indexPath)
{
//    brls::Logger::info("Item Index(" + std::to_string(index.section) + ":" + std::to_string(index.row) + ") selected.");
    recycler->present(new PokemonView(pokemons[indexPath.row]));
}

// RECYCLER VIEW

RecyclingListTab::RecyclingListTab()
{
    // Inflate the tab from the XML file
    this->inflateFromXMLRes("xml/tabs/recycling_list.xml");
    
    pokemons.clear();
    pokemons.push_back(Pokemon("001", "Bulbasaur"));
    pokemons.push_back(Pokemon("004", "Charmander"));
    pokemons.push_back(Pokemon("007", "Squirtle"));
    pokemons.push_back(Pokemon("011", "Metapod"));
    pokemons.push_back(Pokemon("014", "Kakuna"));
    pokemons.push_back(Pokemon("017", "Pidgeotto"));
    pokemons.push_back(Pokemon("021", "Spearow"));
    pokemons.push_back(Pokemon("024", "Arbok"));
    pokemons.push_back(Pokemon("027", "Sandshrew"));

    recycler->estimatedRowHeight = 70;
    recycler->registerCell("Header", []() { return RecyclerHeader::create(); });
    recycler->registerCell("Cell", []() { return RecyclerCell::create(); });
    recycler->setDataSource(new DataSource());
}

brls::View* RecyclingListTab::create()
{
    // Called by the XML engine to create a new RecyclingListTab
    return new RecyclingListTab();
}
