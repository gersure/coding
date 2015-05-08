// (C) 2007-2010 Taobao Inc.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation.
//
// Version: $Id$
//
// Authors: duolong <duolong@taobao.com>

#ifndef TBSYS_MUTEX_H
#define TBSYS_MUTEX_H 

#include <pthread.h>
#include "third_part/tb_common_utils/tbsys/public/lock.h"
#include "third_part/tb_common_utils/tbsys/public/thread_exception.h"

namespace tbutil {

// @brief ������,��ʵ��Ϊ�򵥵����ݽṹ
// Mutex�ǵݹ���,ʹ��ʱ��Ҫע�����¼���:
// 1.��Ҫ��ͬһ�߳��еڶ��ε���lock
// 2.���Ƿ������õ��̳߳���ĳ�������壬����Ҫ��Ըû��������unlock                                                          
class Mutex {
 public:
  typedef LockT<Mutex> Lock;
  typedef TryLockT<Mutex> TryLock;

  Mutex();
  ~Mutex();
  // @brief lock �������Ի�ȡ�����塣����������Ѿ���ס�����ͻ���𷢳�
  // ���õ��̣߳�calling thread����ֱ���������ÿ���Ϊֹ��һ��������
  // �õ��̻߳���˻����壬���þͻ���������
  void lock() const;
  // @brief tryLock �������Ի�ȡ�����塣�����������ã�������ͻ���
  // ס�������þͻ᷵��true����������߳���ס�˻����壬���÷���false
  bool tryLock() const;
  // @brief unlock �������������ļ���
  void unlock() const;
  // @brief �Ƿ��Ѿ��������
  bool willUnlock() const;

 private:
  Mutex(const Mutex&);
  Mutex& operator=(const Mutex&);
  struct LockState {
    pthread_mutex_t* mutex;
  };
  void unlock(LockState&) const;
  void lock(LockState&) const;
  mutable pthread_mutex_t _mutex;
  friend class Cond;
};
}  // namespace tbutil
#endif
