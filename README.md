# MACRO11
MACRO11 is an PDP-11 cross assembler, written in C, originally by Richard Krebiehls in 2008.  This copy was forked from Joerge Hoppe's version on GitHub and further modified with additional changes by Rhialto and Spare Time Gizmos.

  Spare Time Gizmos changes include

  * Set a title string via .TITLE and display it in the listing.
  * Set a subtitle string via .SBTTL and display that too.
  * Generate a table of contents from the subtitles.
  * Make .PAGE actually start a new listing page.
  * Allow form feed in source file to start a new listing page.
  * Add the current time and date to the listing.
  * Miscellaneous fixes to compile under Visual Studio 2022.
