#ifndef BT4PRODUCER
#define BT4PRODUCER

#include <decaf/lang/Thread.h>
#include <decaf/lang/Runnable.h>
#include <decaf/util/concurrent/CountDownLatch.h>
#include <decaf/lang/Integer.h>
#include <decaf/lang/Long.h>
#include <decaf/lang/System.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <memory>

using namespace decaf::util::concurrent;
using namespace decaf::util;
using namespace decaf::lang;
using namespace std;

namespace Sascar {

class BT4Producer : public Runnable
{
	public:
		BT4Producer();
		virtual ~BT4Producer();
		virtual void run();
		void close();

	private:
		void cleanup();
		void createMessage();
		BT4Producer(const BT4Producer&);
		BT4Producer& operator=(const BT4Producer&);
};

} // namespace

#endif // BT4PRODUCER
