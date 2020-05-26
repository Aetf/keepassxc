# Auto-Type on Wayland

This is a very hacky implementation of AutoType on Wayland.
Because there is no standard interfaces to simulate user input or enumerate windows,
this implementation directly talks to KWin using `KWayland`, which implements several
wayland extension that has these feature.

It has the following limitations:

* Only works with KWin, due to the use KWin specific wayland extensions
* Uncommon chars probably will not work. Anything that can not be directly entered on
keyboard beyond SHIFT is untested and likely doesn't work.
