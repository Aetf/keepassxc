# Auto-Type on Wayland

This is a very hacky implementation of AutoType on Wayland.
Because there is no standard interfaces to simulate user input or enumerate windows,
this implementation directly talks to KWin using `KWayland`, which implements several
wayland extension that has these feature.

This however does not mess with uinput directly or require root permission,
and everything will just work if you are under KWin.

The code should be general enough to handle protocols in other window managers should
they implement and expose something similar.


## limitations

* Only works with KWin, due to the use KWin specific wayland extensions
* Uncommon chars probably will not work. Anything that can not be directly entered on
keyboard beyond SHIFT is untested and likely doesn't work.

## Required feature from compositor

| Compositor | Input Simulation | Window enumeration |
| :--------: | :--------------: | :----------------: |
| KDE (KWin) | Wayland extension: org_kde_fake_input (KWayland::Client::FakeInput) | Wayland extension: org_kde_plasma_window_management (KWayland::Client::PlasmaWindowManagement) |
| Gnome (Mutter) | [Ponytail](https://gitlab.gnome.org/ofourdan/gnome-ponytail-daemon) | ? |
| wlroot based | xdg-portal-desktop RemoteDesktop API [wlr implementation](https://github.com/emersion/xdg-desktop-portal-wlr)) | ? |
