/*
 * Copyright (C) 2024-2025 Yury Bobylev <bobilev_yury@mail.ru>
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef MLEXCEPTION_H
#define MLEXCEPTION_H

#include <string>

/*!
 * \brief The MLException class.
 *
 * This class is used as exception to indicate various errors.
 *
 * \deprecated This class is deprecated and will be removed in future releases.
 * MLBookProc uses std::exception for exceptions now.
 */
class __attribute__((deprecated)) MLException
{
public:
  /*!
   * \brief MLException constructor.
   */
  MLException();

  /*!
   * \brief MLException constructor.
   * \param msg message to be printed.
   */
  MLException(const std::string &msg);

  /*!
   * \brief MLException copy constructor.
   */
  MLException(const MLException &other);

  /*!
   * \brief MLException move constructor.
   */
  MLException(MLException &&other);

  /*!
   * \brief operator =
   */
  MLException &
  operator=(const MLException &other);

  /*!
   * \brief operator =
   */
  MLException &
  operator=(MLException &&other);

  /*!
   * \brief Returns \a true if MLException contains message.
   */
  operator bool();

  /*!
   * \brief Returns error message.
   * \return Error info message.
   */
  std::string
  what();

private:
  std::string msg;
};

#endif // MLEXCEPTION_H
