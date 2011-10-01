#include "unix.hpp"

#include <sys/time.h>
#include <stdio.h>
#include <errno.h>
#include <iostream>
#include <unistd.h>

unsigned long long timer_precision()
{
	return 1000000ull;
}

unsigned long long get_time()
{
	timeval tv = {0, 0};
	gettimeofday( &tv, 0 );

	unsigned long long ret = static_cast<unsigned long long>(tv.tv_sec) * 1000 * 1000 + tv.tv_usec;
	return ret;
}

void console_init()
{
	setvbuf(stdout, NULL, _IONBF, 0);
}

void init_rw_lock( rwlock& l )
{
	pthread_rwlock_init( &l, 0 );
}

scoped_shared_lock::scoped_shared_lock( rwlock& l )
	: l_( l )
{
	pthread_rwlock_rdlock( &l_ );
}

scoped_shared_lock::~scoped_shared_lock()
{
	pthread_rwlock_unlock( &l_ );
}

scoped_exclusive_lock::scoped_exclusive_lock( rwlock& l )
	: l_( l )
{
	pthread_rwlock_wrlock( &l_ );
}

scoped_exclusive_lock::~scoped_exclusive_lock()
{
	pthread_rwlock_unlock( &l_ );
}

condition::condition()
	: signalled_(false)
{
	pthread_cond_init( &cond_, 0 );
}


condition::~condition()
{
	pthread_cond_destroy( &cond_ );
}

void condition::wait( scoped_lock& l )
{
	if( signalled_ ) {
		signalled_ = false;
		return;
	}
	int res;
	do {
		res = pthread_cond_wait( &cond_, &l.m_.m_ );
	}
	while( res == EINTR );
	signalled_ = false;
}

typedef char static_assertion_sizeof_tv_nsec[(sizeof(timespec::tv_nsec)==8)?1:-1];

void condition::wait( scoped_lock& l, unsigned long long timeout )
{
	if( signalled_ ) {
		signalled_ = false;
		return;
	}
	int res;
	do {
		timeval tv = {0, 0};
		gettimeofday( &tv, 0 );

		timespec ts;
		ts.tv_sec = tv.tv_sec + timeout / timer_precision();
		ts.tv_nsec = tv.tv_usec + (timeout % timer_precision()) * 1000000 / timer_precision();
		if( ts.tv_nsec > 1000000000ll ) {
			++ts.tv_sec;
			ts.tv_nsec -= 1000000000ll;
		}
		res = pthread_cond_timedwait( &cond_, &l.m_.m_, &ts );
	}
	while( res == EINTR );
	signalled_ = false;
}

void condition::signal( scoped_lock& /*l*/ )
{
	signalled_ = true;
	pthread_cond_signal( &cond_ );
}

thread::thread()
	: t_()
{
}

thread::~thread()
{
	join();
}

namespace {
extern "C" {
static void* run( void* p ) {
	reinterpret_cast<thread*>(p)->onRun();
	return 0;
}
}
}

void thread::spawn()
{
	join();
	t_ = new pthread_t;
	pthread_create( t_, 0, &run, this );
}


void thread::join()
{
	if( !t_ ) {
		return;
	}
	pthread_join( *t_, 0 );
	delete t_;
	t_ = 0;
}


scoped_lock::scoped_lock( mutex& m )
	: m_(m), locked_()
{
	pthread_mutex_lock( &m.m_ );
	locked_ = true;
}


scoped_lock::~scoped_lock()
{
	if( locked_ ) {
		pthread_mutex_unlock( &m_.m_ );
	}
}


void scoped_lock::lock()
{
	pthread_mutex_lock( &m_.m_ );
	locked_ = true;
}


void scoped_lock::unlock()
{
	pthread_mutex_unlock( &m_.m_ );
	locked_ = false;
}


bool thread::spawned()
{
	return t_;
}

mutex::mutex()
{
	pthread_mutex_init( &m_, 0 );
}

mutex::~mutex()
{
	pthread_mutex_destroy( &m_ );
}


int get_cpu_count()
{
	return sysconf(_SC_NPROCESSORS_ONLN);
}


int get_system_memory()
{
	uint64_t pages = sysconf(_SC_PHYS_PAGES);
	uint64_t page_size = sysconf(_SC_PAGE_SIZE);
	return pages * page_size / 1024 / 1024;
}
