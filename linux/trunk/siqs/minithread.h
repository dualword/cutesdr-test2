//
//  sdriq-ftdi: An attempt to create a Unix server for RF-SPACE SDR-IQ 
//
//  Copyright (C) 2012 Andrea Montefusco IW0HDV
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.
//


#if !defined __MINITHREAD_H__
#define      __MINITHREAD_H__

#include <pthread.h>

/**
 *  Mini Thread
 *
 *  Minimalistic implementation of a pthread wrapper
 */

class MiniThread
{
public:
    MiniThread (): thread_run(-1) {}
    ~MiniThread () {}

    virtual int threadproc () = 0;    // pure virtual method to be implemented 
                                      // in derived classes

private:
    pthread_t thread_id;
    int       thread_run;
    int       stop_f;

    static void *thread (void *arg)
    {
        MiniThread *p = (MiniThread *) arg;
        p->threadproc ();
        return p;
    }

public:
    int have_to_stop () { return !stop_f ; }

    int run() 
    {
        stop_f = 0;
        int rc = pthread_create (&thread_id,NULL, thread,(void *)this);
        if(rc<0) {
            throw "pthread_create MiniThread failed";
        } else 
            thread_run = 0;
        return 0;
    }
    int wait ()
    {
        void *ret;

        if (thread_run != -1) {
            int rc = pthread_join (thread_id, &ret);

            thread_run = -1;

            return rc;
        }
        return -1;
    }
    int stop ()
    {
        if (thread_run != -1) {
            stop_f = -1;
            return wait();
        }
        return -1;
    }

};

#endif
