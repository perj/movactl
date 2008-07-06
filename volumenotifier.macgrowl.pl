#!/usr/pkg/bin/perl

use strict;
use Mac::Growl ':all';

my @notifications = (
	'volume'
);

RegisterNotifications("Marantz", \@notifications, \@notifications);

open MAR, '-|', '/usr/local/bin/marantz listen volume';

while (<MAR>) {
	/^volume (.*)/ or next;

	PostNotification("Marantz", 'volume', 'Volume', "$1 dB", 0, 0, '/usr/local/share/sound-icon.png');
}
