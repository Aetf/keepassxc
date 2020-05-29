# Auto-Type on Wayland

This is a very hacky implementation of AutoType on Wayland.
Because there is no standard interfaces to simulate user input or enumerate windows,
this implementation directly talks to KWin using `KWayland`, to make use of several
KWin wayland extensions.

This however has the benefit not messing with uinput directly
and everything will just work if you are using KWin.

The code should be general enough to handle protocols in other Wayland compositor should
they implement and expose something similar.

## Implemented features

* Current keyboard state is considered and restored when sending modifiers.
* Keymap is updated from the compositor automatically. No need to manually config.
* Enumeration of window titles and active window.
* No root permission or special permission configuration is needed.

## limitations

* Only works with KWin, due to the use of KWin specific wayland extensions
* Uncommon chars probably will not work. Anything that can not be directly entered on
a US 101 keyboard is untested and likely doesn't work.

## Required features from the compositor

| Compositor | Input Simulation | Window enumeration |
| :--------: | :--------------: | :----------------: |
| KDE (KWin) | Wayland extension: org_kde_fake_input (KWayland::Client::FakeInput) | Wayland extension: org_kde_plasma_window_management (KWayland::Client::PlasmaWindowManagement) |
| Gnome (Mutter) | [Ponytail](https://gitlab.gnome.org/ofourdan/gnome-ponytail-daemon) | ? |
| wlroot based | xdg-portal-desktop RemoteDesktop API [wlr implementation](https://github.com/emersion/xdg-desktop-portal-wlr)) | ? |
