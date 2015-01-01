#include "File.h"

MediaInfoLib::MediaInfo MediaFile::mMInfoHandle;
const wxString musicCols[] = {"Title", "Time", "Artist",
	"Album", "Genre"};
const int musicColSizes[] = {6, 1, 3, 3, 3};

template<>
const int MediaFileT<MusicFile>::mColCount = 5;
template<>
const wxString * MediaFileT<MusicFile>::mColumns = musicCols;
template<>
const int * MediaFileT<MusicFile>::mColSizes = musicColSizes;

void MusicFile::CollectInfo()
{
	wxString path = GetFullPath();
	bool r = mMInfoHandle.Open(path.ToStdWstring());
	//TODO get length and other things from taglib
	if (!r)
		throw MyException("MusicFile::CollectInfo(): failed to initialise tagfile", MyException::NOT_FATAL);

	mColContents[0] = mMInfoHandle.Get(MediaInfoLib::Stream_General,
			0, L"Title").c_str();
	wxString timeStr = mMInfoHandle.Get(MediaInfoLib::Stream_General,
			0, L"Duration/String3").c_str();
	if (timeStr.size() >= 9)
	{
		if (timeStr[0] == '0')
			mColContents[1] = timeStr.substr(4, 4);
		else
			mColContents[1] = timeStr.substr(3, 5);
	}
	mColContents[2] = mMInfoHandle.Get(MediaInfoLib::Stream_General, 
			0, L"Performer");
	mColContents[3] =  mMInfoHandle.Get(MediaInfoLib::Stream_General, 
			0, L"Album");
	mColContents[4] = mMInfoHandle.Get(MediaInfoLib::Stream_General, 
			0, L"Genre");
	mMInfoHandle.Close();
}

File::~File()
{
}


