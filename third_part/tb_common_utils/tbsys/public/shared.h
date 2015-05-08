// (C) 2007-2010 Taobao Inc.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation.
//
// Version: $Id$
//
// Authors: duolong <duolong@taobao.com>

#ifndef TBSYS_SHARED_H 
#define TBSYS_SHARED_H 

#include "third_part/tb_common_utils/tbsys/public/mutex.h"

namespace tbutil {

// @brief SimpleShared���ṩ�򵥵����ü���
class SimpleShared {
 public:
  SimpleShared();
  SimpleShared(const SimpleShared&);
  virtual ~SimpleShared() { }
  SimpleShared & operator = (const SimpleShared&) {
    return *this;
  }

  void __incRef() {
    assert(_ref >= 0);
    ++_ref;
  }

  void __decRef() {
    assert(_ref > 0);
    if (--_ref == 0) {
      if (!_noDelete) {
        _noDelete = true;
        delete this;
      }
    }
  }

  int __getRef() const {
    return _ref;
  }

  void __setNoDelete(bool b) {
    _noDelete = b;
  }

 private:
  int _ref;
  bool _noDelete;
};

// @brief Shared �ṩ�򵥵����ü���,��Ҫ��������ָ��
// ���Ҫ��������ָ�룬�û�������Ҫ�̳д��� 
class Shared {
 public:
  Shared();
  Shared(const Shared&);
  virtual ~Shared() { }
  Shared & operator=(const Shared&) {
    return *this;
  }
  // @brief �������ü���
  virtual void __incRef();
  // @brief �������ü���
  virtual void __decRef();
  // @brief ��ȡ��ǰ���ü���
  virtual int __getRef() const;
  // @brief ����__noDelete��־
  // true: �����ü���Ϊ0ʱ��ɾ��������Ķ���
  // false: �����ü���Ϊ0ʱɾ��������Ķ��� 
  // @param bool
  virtual void __setNoDelete(bool);

 protected:
  int _ref;
  bool _noDelete;
  Mutex _mutex;
};
}  // namespace tbutil
#endif
