#pragma once

#if defined (_WIN32)
	#define WINDOWS
#elif (__linux__) || defined (__linux)
	#define LINUX
#elif defined (__ANDROID__)
	#define ANDROID
#endif

#if defined (WINDOWS)
	#ifdef DIRECTORYWATCHERDLL_EXPORTS  
		#define DW_EXPORT __declspec(dllexport)   
	#else  
		#define DW_EXPORT __declspec(dllimport)   
	#endif
#elif defined (LINUX)
	#define DW_EXPORT __attribute__ ((dllexport))

	#if __GNUC__ >= 4
    	#define DW_EXPORT __attribute__ ((visibility ("default")))
	#else
		#define DW_EXPORT
	#endif
#elif defined (ANDROID)
	#define DW_EXPORT
#endif

#include <string>
#include <vector>
#include <thread>

#include "CancelationToken.h"
#include "StatusFlag.h"

/*	This is an implementation of filesystem monitor concept that was created to report filesystem 
	changes to application.

	Currently DirectoryWatcher class is able to provide information about new file/directory addition,
	content modification and deletion via callback.

	Limitations:
	- DirectoryWatcher doesn't handle file/directory renaming events directly
	- Locks main thread on DirectoryWatcher's worker task start and stop but assures their creation 
	  and destruction

	Done's: 
	- win32 API implementation of filesystem monitor
	- win32 API dependency is as minimal as possible
	- DirectoryWatcher is a consumable DLL
	- Persistent usability in tests

	ToDo's:
	- Linux implemention of filesystem monitor

	Blog post that led me right way https://qualapps.blogspot.lt/2010/05/understanding-readdirectorychangesw.html
	(by Jim Beveridge)

	This utility was made in order to get more experience with C++, learn more about win32 API, to 
	consolidate practical use of multithreaded programming. 

	Task was given by my mentor Mantas Ragauskas 2017-03-23.

	Made by Egidijus Lileika 2017-04-19
*/

/**************************************************************************************************/
/*  Describes the type of change or directory watcher's operation failure in specified directory  */
/**************************************************************************************************/
enum DW_EXPORT CallbackType {
	Added,
	Deleted,
	Modified,
	FailedToWatch
};

/**************************************************************************************************/
/*  Describes DirectoryWatcher status  */
/**************************************************************************************************/
enum DW_EXPORT DirectoryWatcherStatus {
	Idle,
	Watching
};

/**************************************************************************************************/
/*              Callback type definition used by DirectoryWatcher class                           */
/**************************************************************************************************/
typedef DW_EXPORT std::function<void(std::string& directory, CallbackType type, const std::string& details)> dw_callback;

/**************************************************************************************************/
/*              Type that defines worker thread that observes specified directory                 */
/**************************************************************************************************/
typedef DW_EXPORT std::pair<std::thread, std::string> watcher;

/**************************************************************************************************/
/*                                 DirectoryWatcher class                                         */
/**************************************************************************************************/
class DW_EXPORT DirectoryWatcher
{
	//---------------------------------------------------------------------------------------------
	//  Public Constructors and Destructors
	//---------------------------------------------------------------------------------------------
public:
	DirectoryWatcher(dw_callback callback);
	DirectoryWatcher(std::string directory, dw_callback callback);
	DirectoryWatcher(std::vector<std::string> directories, dw_callback callback);
	~DirectoryWatcher();

	//---------------------------------------------------------------------------------------------
	//  Public Methods
	//---------------------------------------------------------------------------------------------
public:
	/* Returns true if DirectoryWatcher starter successfully, otherwise - false */
	bool Watch(bool watchSubDir);

	/* Sets global cancelation token and cancels all directory watcher's worker threads */
	void Stop();

	/* Launches additional thread to watch specified directory */
	void AddDir(std::string directory);

	/* Sets individual cancelation token and cancels directory watcher's worker thread that 
	   watches specified directory */
	void RemoveDir(std::string directory);

	/* Returns DirectoryWacher's watch status*/
	DirectoryWatcherStatus GetStatus();

	//---------------------------------------------------------------------------------------------
	//  Private Members
	//---------------------------------------------------------------------------------------------
private:
	std::vector<std::string> m_directories;
	std::vector<watcher> m_watchers;
	CancelationToken m_ct;
	bool m_isWatching;
	bool m_watchSubDiretories;
	dw_callback m_callback;
	StatusFlag<DirectoryWatcherStatus> m_statusFlag;
};



