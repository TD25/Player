#include "File.h"

const wxString musicCols[] = {"Title", "Time", "Artist",
	"Album", "Genre"};
const int musicColSizes[] = {6, 1, 3, 3, 3};

template<>
const int FileT<MusicFile>::mColCount = 5;
template<>
const wxString * FileT<MusicFile>::mColumns = musicCols;
template<>
const int * FileT<MusicFile>::mColSizes = musicColSizes;

void MusicFile::CollectInfo()
{
	//TODO get length and other things from taglib
	if (mTagFile.isNull() || !mTagFile.tag())
		throw MyException("MusicFile::CollectInfo(): failed to initialise tagfile", MyException::FATAL_ERROR);

	TagLib::Tag * tag = mTagFile.tag();
	mColContents[0] = tag->title().toCString(false);
	int seconds = mTagFile.audioProperties()->length() % 60;
	int minutes = (mTagFile.audioProperties()->length()-seconds) % 60;
	mColContents[1].Printf("%d.%d", minutes, seconds);
	mColContents[2] = tag->artist().toCString(false);
	mColContents[3] = tag->album().toCString(false);
	mColContents[4] = tag->genre().toCString(false);
}

File::~File()
{
}


