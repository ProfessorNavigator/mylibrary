/*
 * ModelBoxes.cpp
 *
 *  Created on: 19 мар. 2023 г.
 *      Author: yury
 */

#include <ModelBoxes.h>

ModelBoxes::ModelBoxes(std::string &menu_line)
{
  this->menu_line = Glib::ustring(menu_line);
}

ModelBoxes::~ModelBoxes()
{
  // TODO Auto-generated destructor stub
}

Glib::RefPtr<ModelBoxes>
ModelBoxes::create(std::string menu_line)
{
  return Glib::make_refptr_for_instance<ModelBoxes>(new ModelBoxes(menu_line));
}
