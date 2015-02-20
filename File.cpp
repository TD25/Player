#include "File.h"

using namespace MediaInfoNamespace;

MediaInfo MediaFile::mMInfoHandle;
static const wxString fileCols[] = { "Title" };
static const wxString mediaCols[] = { "Title", "Time" };
static const wxString musicCols[] = {"Title", "Time", "Artist",
	"Album", "Genre"};
static const int musicColSizes[] = {6, 1, 3, 3, 3};
static const wxString videoCols[] = {"Title", "Time", "Format", 
	"Dimensions"};
static const int videoColSizes[] = {6, 1, 3, 2};


File::FileInfo File::mInfo = FileInfo("none", fileCols, 1); 
File::FileInfo MediaFile::mMedInfo = FileInfo("mediaFile", mediaCols, 3);
File::FileInfo MusicFile::mMusicInfo = FileInfo("Music", musicCols, 5,
	musicColSizes);
File::FileInfo VideoFile::mVidInfo = FileInfo("Video", videoCols, 4,
	videoColSizes);

void File::CollectInfo()
{
	mColContents[0] = GetName();
}
MediaFile::MediaFile() : File()
{
	mMInfoHandle.Option(L"ParseSpeed", L"0");
}

MediaFile::MediaFile(const wxFileName & wxfilename) : File(wxfilename) 
{

	mMInfoHandle.Option(L"ParseSpeed", L"0");
}

MediaFile::MediaFile(const wxString & filename) : File(filename) 
{
	mMInfoHandle.Option(L"ParseSpeed", L"0");
}

void MediaFile::StoreTitleAndTime()
{
	assert(mMInfoHandle.IsReady());
	mColContents[0] = mMInfoHandle.Get(Stream_General,
			0, L"Title").c_str();
	wxString timeStr = mMInfoHandle.Get(Stream_General,
			0, L"Duration").c_str();
	long mSecs;
	timeStr.ToLong(&mSecs, 10);
	int seconds = (mSecs / 1000);
	int secs = seconds % 60;
	int minutes = (seconds - secs) / 60;
	mColContents[1] = FormatTime(minutes, secs);
}

void MediaFile::CollectInfo()
{
	wxString path = GetFullPath();
	wxCriticalSectionLocker locker(mMInfoCS);
	wxCriticalSectionLocker dLocker(mCS);
	bool r = mMInfoHandle.Open(path.ToStdWstring());
	if (!r)
		throw MyException("MusicFile::CollectInfo(): failed to initialise tagfile", MyException::NOT_FATAL);
	StoreTitleAndTime();
	mMInfoHandle.Close();
}

void MusicFile::CollectInfo()
{
	wxString path = GetFullPath();
	wxCriticalSectionLocker locker(mMInfoCS);
	wxCriticalSectionLocker dLocker(mCS);
	bool r = mMInfoHandle.Open(path.ToStdWstring());
	if (!r)
		throw MyException("MusicFile::CollectInfo(): failed to initialise tagfile", MyException::NOT_FATAL);

	StoreTitleAndTime();
	mColContents[2] = mMInfoHandle.Get(Stream_General, 
			0, L"Performer");
	mColContents[3] =  mMInfoHandle.Get(Stream_General, 
			0, L"Album");
	mColContents[4] = mMInfoHandle.Get(Stream_General, 
			0, L"Genre");
	mMInfoHandle.Close();
}

File::~File()
{
}

wxString MediaFile::GetTitle() const
{
	wxCriticalSectionLocker locker(mCS);
	return mColContents[0];
}

wxFileOffset MediaFile::GetLength() const
{
	wxCriticalSectionLocker locker(mCS);
	FileInfo & i = GetFileInfo();
	if (mColContents[1].size() <= 0)
		return 0;
	long min;
	mColContents[1].ToLong(&min);
	long seconds; 
	mColContents[1].Mid(mColContents[1].find(':') + 1)
		.ToLong(&seconds);
	//converting to miliseconds
	return (min * 60 + seconds) * 1000;
}

void VideoFile::CollectInfo()
{
	FileInfo & i = GetFileInfo();
	wxCriticalSectionLocker locker(mMInfoCS);
	wxCriticalSectionLocker dLocker(mCS);
	wxString path = GetFullPath();
	bool r = mMInfoHandle.Open(path.ToStdWstring());
	if (!r)
		throw MyException("MusicFile::CollectInfo(): failed to initialise tagfile", MyException::NOT_FATAL);

	
	StoreTitleAndTime();
	wxString str;
	mColContents[2] = mMInfoHandle.Get(Stream_Video,
			0, L"Format").c_str();
	str = mMInfoHandle.Get(Stream_Video,
		0, L"Width").c_str();	
	wxString height = mMInfoHandle.Get(Stream_Video,
			0, L"Height").c_str();
	str.Printf("%s x %s", str, height);
	mColContents[3] = str;
	mMInfoHandle.Close();
}
