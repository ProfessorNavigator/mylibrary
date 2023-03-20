/*
 * ModelBoxes.h
 *
 *  Created on: 19 мар. 2023 г.
 *      Author: yury
 */

#ifndef INCLUDE_MODELBOXES_H_
#define INCLUDE_MODELBOXES_H_

#include <gtkmm.h>

class ModelBoxes : public Glib::Object
{
public:
  virtual
  ~ModelBoxes();
  Glib::ustring menu_line;
  static Glib::RefPtr<ModelBoxes>
  create(std::string menu_line);
protected:
  ModelBoxes(std::string &menu_line);
};

#endif /* INCLUDE_MODELBOXES_H_ */
