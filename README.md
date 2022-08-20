# PicMan
Picture Manager, a program designed to manage several different directories of JPEG images with the help of MySQL.

Written in c++ on Windows 10.  Uses 2 custom libaries, AWindowsLib and AWindowsProse both on GitHub under my account.
PicMan uses mysql-connector-odbc-8.0.29-winx64.msi for connecting to the database.

Picture Manager was written and developed to sort through hundreds of images of a specific subject matter coming from many different sources. It's used to group pictures under a directory together into sets.  It can test for exact duplication. Pictures can be sorted by color to help find additional pictures of a set or to weed out near duplicate images. Images can be tagged with either global or folder level hash tags to help with organization.

1) The program only works with directories containing pictures directly under an established base directory. The program will not look further under folders in the base directory for additional directories.  Several base directories can be added and maintained. Only one base directory can be opened at a time.  Only one folder can be opened at a time.

2) The program only works with JPEG images saved with the extesion ".jpg".

3) The program will search for and show organized groups of pictures with file names that contain an "_" followed by a number. For example: Tree_001.jpg Tree_002.jpg Tree_003.jpg Tree_0004.jpg will be shown as a group of files named "Tree". Spaces are allowed in file names.

4) The program uses a file extension ".ord", short for order, when renaming sets of files to ensure no naming conflicts occur when organizing files.  If an error occurs during processing any file stuck with an ".ord" extension  can be changed to back ".jpg" and the image will not be lost.

5) Files are never deleted from storage, they are moved to the recycle bin.

6) The program will not attempt to discover additional pictures under sub-directories. A design effort was made to allow for this behavior but complications were discovered during coding such as how to handle moving files between sub-directories. An executive decision was made to keep the system simple and two dimensional.

7) Currently the program re-numbers group files with 3 digits allowing for 999 pictures per group. This could be changed in the future to a configurable length.

8) During coding and testing I worked with dirctories containing nearly 2000 images. As designed, the program loads all image information and thumbnails of a specific directory or folder into memory. There will be a limit to the number of jpgs the program can handle at one time.  For a workaround a large directory could be split into several smaller directories but checking for exact image duplication across these would not be possible. 

9) Large images such as a scanned image with a DPI of 600 will take time to display in preview panes. If the high DPI isn't vital the loading will be much faster if the image is reduced to a DPI of 100 or less. 

10) MariaDB, an open source MySQL database was used in development of the program.  Any MySQL database should work but the supplied stored procedures would have to be adapted if the database is not MariaDB.

11) A different program could be written for dealing with massive numbers of JPGs, millions, allowing for automatic sub directory processiong.  It would not attempt to load an entire catalog of images into memory and must rely on text searching by hash tag values and filenames. Perhaps the duplication feature, the ability to detect exact image duplication could be implemented. Such a program would not be useful to visually sort through images.

12) The directory containing PicMan.EXE must also contain the file "ProseData.dat".  This file contains the text of every element of the graphic user interface.  The utility AWindowProse can be used to add additional languages to the ProseData.dat file.  

13) The Picture Manager program, database and ProseData.dat all work with the UTF-16, the wide 16 bit character set.

14) Date and time fields on the database are stored as local time of the computer or file share where the images are stored. Dates are not stored in UTC or Greenwich mean time.  The database value will equal the value of the file's last write date on the storage device.  

15) The program relies on a specific "incoming" / "import" directory to allow for additional images to added to a folder.  The contents of the import directory are constantly refreshed and shown in the "import" list when folders are opened for viewing. Program operations allow for a few different ways to merge images into the open folder such as adding to a new group described in section 3.

16) Two types of tags are available to help organize files.  Global hashtags such as "Favorite" or "Autograph" can be applied to any picture across folders and all pictures with a global hashtag can be viewed at the same time.   Individual folders under the base directory can contain folder level HashTags specific only to that folder. If a base directory contains too many pictures with a specific global hash tag it could cause the program to crash if out of memory when an attempt were made to view them all, there would be a limit to the number of pictures assigned with a specific global hash tag.

17) Folder Hashtags can be combined together.  For example the tags "Gown", "Black", "Red", "Sequins" can be added to a folder then combined together on a image to form Gown (Black, Sequins). "Gown" is chosen first then "Black" and "Sequins" are added to it. The assignments can be many levels deep such as Gown (Black, Sequins (Red)). When viewing the hash tag tree of a folder the list of hash tags can be toggled on to limit the the view to pictures containing those specific hash tags. After pictures in a folder are organized with hash tags the entire folder contents can be renamed to reflect the hash tags assigned to each image.

18) The program extracts color information from the 128 x 128 thumbnail, stored on the database as a compressed PNG file, each time a folder is opened.  The extracted color information is not written to the database.  The color information is used to determine exact picture matches and can be used to sort pictures in the viewing list. Pictures can be flipped horizontally or vertically and still show up as duplicates. The program can check for duplicates in the import directory, section 15. Only exact duplication can be reported, if an image has been cropped or changed with filters in any way it will not show up as duplicate. Sort lists by color RGB or by Average Edge Color to visually check for duplicates that are not exact.  

19) Sorting by RGB Color or Average Edge Color can come close to showing groups of pictures of the same scene but it's never exact.  No type of image interpretation such as facial recognition is performed.

