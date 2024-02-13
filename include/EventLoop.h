
//
// Created by xy on 2024-02-13.
//

#ifndef NET__EVENTLOOP_H_
#define NET__EVENTLOOP_H_

#include "Channel.h"

class EventLoop {
 public:
  void updateChannel(Channel *ch);
  void removeChannel(Channel *ch);
};

#endif //NET__EVENTLOOP_H_
