/// \file
/// \brief Template base class for defining singleton objects.
///
/// \copyright
///   Copyright 2019 by Google LLC. All Rights Reserved.
///
/// \copyright
///   Licensed under the Apache License, Version 2.0 (the "License"); you may
///   not use this file except in compliance with the License. You may obtain a
///   copy of the License at
///
/// \copyright
///   http://www.apache.org/licenses/LICENSE-2.0
///
/// \copyright
///   Unless required by applicable law or agreed to in writing, software
///   distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
///   WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
///   License for the specific language governing permissions and limitations
///   under the License.

#ifndef ASH_SINGLETON_H_
#define ASH_SINGLETON_H_

namespace ash {

/// Singleton class; useful as a base class too.
template <typename T>
struct singleton {
  static T& get() {
    static T instance;
    return instance;
  }
};

}  // namespace ash

#endif  // ASH_SINGLETON_H_
