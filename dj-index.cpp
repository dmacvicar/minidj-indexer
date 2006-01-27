/*
 *     Typhoon My MiniDj Database Indexer
 *     
 *     Copyright (c) 2006 by Duncan Mac-Vicar Prett <duncan@kde.org>
 *
 *     thanks to Michael, for figuring out the .ldd indexes format
 *     
 *     Viva Chile Mierda!
 *
 *     *************************************************************************
 *     *                                                                       *
 *     * This program is free software; you can redistribute it and/or modify  *
 *     * it under the terms of the GNU General Public License as published by  *
 *     * the Free Software Foundation; either version 2 of the License, or     *
 *     * (at your option) any later version.                                   *
 *     *                                                                       *
 *     *************************************************************************
 */

#include <iostream>
#include <qdir.h>
#include <qfile.h>
#include <qcstring.h>
#include <qvaluelist.h>
#include <qmap.h>

#include <tag.h>
#include <fileref.h>
#include <string.h>

#define DEVICE_MNT_POINT "/mnt/tmp"

#define LOG std::cout
//#define LOG log_file

int g_song_index = 0;

struct Song;
typedef QValueList<Song> SongList;
typedef QMap<QString, SongList> SongIndex;
typedef QValueList<QString> StringList;

static void writeRecord( QDataStream &device, const char *data, uint data_len, uint record_len)
{
  int k;
  // write the data
  for(k=0; k < data_len; k++)
    device << (Q_INT8) data[k];
  // fill with 0
  for(k=0; k < record_len - data_len; k++)
    device << Q_INT8(0);
}

static void insertIfNotExists( QValueList<QString> &list, QString element)
{
  if ( ! list.contains(element) )
    list.append(element);
}

struct Song
{
  uint index;
  QString filename;
  QString title;
  QString album;
  QString artist;
  QString genre;
  QString year;
  QString track;
};

struct MiniDJ
{
  // cache for future key sorting
  StringList titlesList;
  StringList albumsList;
  StringList artistsList;
  StringList genresList;
  StringList yearsList;
  // all songs
  SongList songs;  
  // indexes 
  SongIndex titleIndex;
  SongIndex albumIndex;
  SongIndex artistIndex;
  SongIndex genreIndex;
  SongIndex yearIndex;
 
  QString toQString( const TagLib::String &string )
  {
    return QString::fromLatin1(string.toCString());
  }
  
  void addSong( const Song &song )
  {
    songs.append(song);
    QString title_key = QString(song.title[0]).upper();
    titleIndex[title_key].append(song);
    artistIndex[song.artist].append(song);
    albumIndex[song.album].append(song);
    yearIndex[song.year].append(song);
    genreIndex[song.genre].append(song);
    std::cout << "Song [" << song.title << "] added to Title: " << title_key.latin1() << std::endl;
    //std::cout << "Song..." << std::endl; 
  }
  
  void writeIndex( const SongIndex &index, const QString filename )
  {
    LOG << "Writing index " << filename << " with " << index.count() << " keys" << std::endl;
    QFile db_file(filename.latin1());
    if ( db_file.open( IO_WriteOnly ) )
    {
      QDataStream file(&db_file);
      file.setByteOrder(QDataStream::LittleEndian);
      //  0-7   numero de registros en k2
      //  8-15    numero de colecciones en el archivo
      //  16-511    mas headers desconocidos
      //  512-(512+8*n-1) Registros de 8 bytes
      //  2 bytes 88 90  (pareciera que son fijos)
      //  2 bytes Id de coleccion partiendo por el 1
      //  2 bytes Offset de su primera cancion en la lista del final 
      //  2 bytes Offset de su ultima cancion en la lista del final 
      //  2 bytes   a0 90 Indica el fin de los registros de colecciones
      //  resto   Ahora vienen registros de 4 bytes indicando las canciones de cada coleccion       (utilizando el indice de k2 que parecieran estar numerados desde 0)
      file << Q_INT64(songs.count());
      file << Q_INT64(index.count());
      // ahora 496 bytes headers desconocidos
      // header is 512 bytes
      int k;
      for (k=0; k< 60; k++)
        file << Q_INT64(0);
      // por cada coleccion
      uint total_songs = 0;
      uint total_collections = 0;
      SongIndex::ConstIterator it = index.begin();
      for( ; it != index.end(); ++it)
      {
        LOG << "... writing index header (collection list)" << std::endl;
        total_collections++;
        SongList indexSongs = it.data();
        uint offset_first = total_songs;
        uint offset_last = offset_first + indexSongs.count();
        total_songs += indexSongs.count();
        // write the marker and collection id
        file << Q_INT8(136) << Q_INT8(144);
        file << Q_INT16(total_collections);
        file << Q_INT16(offset_first) << Q_INT16(offset_last);
      
     }
      
     // write the marker to start enumerating songs
     file << Q_INT16(37024);
      
     for( it = index.begin(); it != index.end(); ++it)
     {
       SongList indexSongs = it.data();
       LOG << "Index " << filename << " has " << indexSongs.count() << " songs" << std::endl;
       for( SongList::ConstIterator sit = indexSongs.begin(); sit != indexSongs.end(); ++sit)
       {
         LOG << "cancion" << std::endl;
         Song curr_song = *sit;
         file << Q_INT16(curr_song.index);
       }
     }

    }
  }
  
  void writeMainDatabase(const QString &dir)
  {
    QFile db_file( (dir + QString::fromLatin1("/mymusic/db/k2.gdd")).latin1());
    if ( db_file.open( IO_WriteOnly ) )
    {
      QDataStream file(&db_file);
      file.setByteOrder(QDataStream::LittleEndian);

      int num_songs = songs.count();
      file << Q_INT64(num_songs);
      file << Q_INT64(0);
      file << Q_INT64(10428);
      file << Q_INT64(276);

      // header is 512 bytes
      int k;
      for (k=0; k< 60; k++)
        file << Q_INT64(0);
      
      for (SongList::ConstIterator it = songs.begin(); it != songs.end(); it++)
      {
        Song song = *it;
        // write the directory (32 bytes)
        writeRecord( file, "\\K2\\", 4, 28);
        // write the filename
        writeRecord( file, song.filename.latin1(), 12, 12);
        // write the title ( 64 bytes )
        writeRecord( file , song.title.latin1(), song.title.length(), 64);
        writeRecord( file , song.album.latin1(), song.album.length(), 64);
        writeRecord( file , song.artist.latin1(), song.artist.length(), 64);
        writeRecord( file , song.genre.latin1(), song.genre.length(), 30);
        writeRecord( file , song.track.latin1(), song.track.length(), 2);     
        writeRecord( file , song.year.latin1(), song.year.length(), 4);
        file << Q_INT64(0);
      }
    } 
  }  
 
  void writeIndexes(const QString &dir)
  {
    QString filename_base = dir + QString::fromLatin1("/mymusic/db");
    writeIndex( titleIndex, filename_base + "/songs.ldd");
    writeIndex( artistIndex, filename_base + "/artist.ldd");
    writeIndex( yearIndex, filename_base + "/year.ldd");
    writeIndex( genreIndex, filename_base + "/genre.ldd");
    writeIndex( albumIndex, filename_base + "/album.ldd");
  }
  
  void scan(const QString &dir)
  {
      QDir d( (dir + QString::fromLatin1("/mymusic/k2")).latin1());
      d.setFilter( QDir::Files | QDir::Hidden | QDir::NoSymLinks );
      d.setSorting( QDir::Size | QDir::Reversed );

      const QFileInfoList *list = d.entryInfoList();
      QFileInfoListIterator it( *list );
      QFileInfo *fi;

      while ( (fi = it.current()) != 0 ) 
      {
        QString name = fi->fileName().upper();

        TagLib::FileRef f(fi->filePath());
        QString title = toQString(f.tag()->title());
        QString artist = toQString(f.tag()->artist());
        QString album = toQString(f.tag()->album());
        QString genre = toQString(f.tag()->genre());

        // convert year to string
        uint year_int = f.tag()->year();
        QString year = QString::number(year_int);
        if( year_int == 0)
          year = "????";

        // convert track to  string
        uint track_int  = f.tag()->track(); 
        QString track = QString::number(track_int);
        if( track_int < 10)
          track = "0" + track;

        Song song;
        song.index = g_song_index++;
        song.filename = name;
        song.title = title;
        song.album = album;
        song.artist = artist;
        song.genre = genre;
        song.year = year;
        song.track = track;
        addSong(song);

        printf( "%s %s %s %s %s %s\n", fi->filePath().latin1(), artist.latin1(), album.latin1(), genre.latin1(), year.latin1(), track.latin1() );

        ++it;
      }
  }

  void index(const QString &dir)
  {
    scan(dir);
    writeMainDatabase(dir);
    writeIndexes(dir);
  }

};

static void showUsage(const char *prog)
{
  LOG << "Usage:" << prog << " mount-point" << std::endl;
  LOG << "Where mount-point is where your device is mounted as msdos" << std::endl;
}

static void showInfo()
{
  LOG << "My Mini Dj database indexer - (c) 2006 Duncan Mac-Vicar P." << std::endl;
}

int main(int argc, char *argv[])
{
  if(argc != 2)
  {
    showInfo();
    showUsage(argv[0]); 
    exit(1);
  }
  else
  {
    if(!QDir(QString::fromLatin1(argv[1]) + QString::fromLatin1("/mymusic/k2")).exists())
    {
      LOG << argv[1] << " is missing mymusic/k2. Check there is a real device mounted there" << std::endl;
      exit(1);
    }  
  }
  
  MiniDJ dj;
  dj.index(QString::fromLatin1(argv[1])); 
  return 0;
}
