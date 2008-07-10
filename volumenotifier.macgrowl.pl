#!/usr/pkg/bin/perl

use strict;
use Mac::Growl ':all';

my @notifications = (
	'volume'
);

RegisterNotifications("Morantz", \@notifications, \@notifications);

open MAR, '-|', '/usr/local/bin/morantz listen volume';

while (<MAR>) {
	/^volume (.*)/ or next;

	PostNotification("Morantz", 'volume', 'Volume', "$1 dB", 0, 0, '/usr/local/share/sound-icon.png');
}
