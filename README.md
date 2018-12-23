# qtimemage
QTimeMage is a time tracking program that doesn't suck! 

Important qualities include:

1) Written in C++ by a seasoned professional software developer.
2) All events are stored using an sqlite3 database.
3) GUI is intuitive, and implemented using Qt5.
4) Effortlessly supports multiple clients, each with multiple projects, subprojects, subsubprojects, ...
5) Simple time sheet reports for week & month, stored in HTML format (easy to email).
6) Not full of bugs - I've been using it for several months, and it doesn't crash or do crazy things.

Please read the INSTALL file before trying to run the program!

DOCUMENTATION

----- First Client Record ----
After initializing the database as described in the INSTALL file, you'll need to
create at least one client. To do this, click on the '+' icon at the left of the
tool bar at the top left of the window. If you hold the button down, a full
client menu will appear, and you can also edit or delete a client.

----- First Project Record ----
Next, you need to create a project. This is done by clicking on the '+' icon
near the top left of the client page. Again, if you hold the button down, the
full project menu will appear. Also, you can right-click anywhere in the project
tree view area.

----- Tracking Time ----
Once you have created a project, and it is selected in the combo box in the tool
bar at the top of the window, you can begin tracking time by clicking on
the "record" button. You may later either pause or stop timekeeping with the
"pause" or "stop" buttons. The difference between pause and stop is that using
"stop" will cause the timesheet report to charge at minimum the client's
'minimum billing quantum' for any start/stop pair of records. The main purpose
of "stop" is to penalize clients who interrupt you frequently for short periods
of time, like phone calls and emails which require prompt attention.

----- Window Close Button ----
Clicking on the window manager 'close' button will cause the main GUI window to
disappear, but the system tray icon will remain. You may reopen the GUI window
at any time by choosing the 'Show GUI' menu item for the system tray icon.

----- Time Sheet Reports ----
You may create either a timesheet report for either a week or month by clicking
on the menu at the top right of the client page. When presented with the Qt
calendar widget, select a date within the week or month you want reported.

----- Project Start/Stop/Pause Records ----
Records from one project can be moved to another project by selecting them, and
then right-clicking and choosing 'Move event to another Project'. At this point
you will be presented with a list of all project to choose from. This feature is
useful if you realize that you have been charging the wrong project for some
period of time.

----- Time Stamps ------------
You may edit a time stamp by right clicking on it, and choosing 'Edit
Timestamp'.

----- Data Persistance --------
Your sqlite3 database is where all important records are stored. The state of
time keeping is determined by the records in the sqlite3 database, regardless of
whether or not QTimeMage happens to be running. If you log out or otherwise
cause QTimeMage to exit, time is still being tracked.
Program settings are written to a file every 5 minutes.

