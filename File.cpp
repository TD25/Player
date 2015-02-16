#include "File.h"

using namespace MediaInfoNamespace;

MediaInfo MediaFile::mMInfoHandle;
static const wxString musicCols[] = {"Title", "Time", "Artist",
	"Album", "Genre"};
static const int musicColSizes[] = {6, 1, 3, 3, 3};
static const wxString videoCols[] = {"Title", "Time", "Format", 
	"Dimensions"};
static const int videoColSizes[] = {6, 1, 3, 2};

template<>
const int MediaFileT<MusicFile>::mColCount = 5;
template<>
const wxString * MediaFileT<MusicFile>::mColumns = musicCols;
template<>
const int * MediaFileT<MusicFile>::mColSizes = musicColSizes;
template<>
const int MediaFileT<VideoFile>::mColCount = 4;
template<>
const wxString * MediaFileT<VideoFile>::mColumns = videoCols;
template<>
const int * MediaFileT<VideoFile>::mColSizes = videoColSizes;

MediaFile::MediaFile()
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

void MusicFile::CollectInfo()
{
	wxString path = GetFullPath();
	bool r = mMInfoHandle.Open(path.ToStdWstring());
	if (!r)
		throw MyException("MusicFile::CollectInfo(): failed to initialise tagfile", MyException::NOT_FATAL);

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

wxFileOffset MediaFile::GetLength() const
{
	bool r = mMInfoHandle.Open(GetFullPath().ToStdWstring());		
	if (!r)
		throw MyException("MediaFile::GetLength(): MediaInfo failed\
				to open file", MyException::NOT_FATAL);
	wxString str = mMInfoHandle.Get(Stream_General,
			0, L"Duration").c_str();
	long l;
	str.ToLong(&l, 10);
	mMInfoHandle.Close();
	return l;
}

void VideoFile::CollectInfo()
{
	wxString path = GetFullPath();
	
	wxMessageOutputDebug().Printf("opening " + path);
	bool r = mMInfoHandle.Open(path.ToStdWstring());
	wxMessageOutputDebug().Printf("opened");
	if (!r)
		throw MyException("MusicFile::CollectInfo(): failed to initialise tagfile", MyException::NOT_FATAL);

	mColContents[0] = mMInfoHandle.Get(Stream_General,
			0, L"Title").c_str();
	wxString str = mMInfoHandle.Get(Stream_General,
			0, L"Duration").c_str();
	long mSecs;
	str.ToLong(&mSecs, 10);
	int seconds = (mSecs / 1000);
	int secs = seconds % 60;
	int minutes = (seconds - secs) / 60;
	mColContents[1] = FormatTime(minutes, secs);
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
