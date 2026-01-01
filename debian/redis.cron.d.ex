#
# Regular cron jobs for the redis package.
#
0 4	* * *	root	[ -x /usr/bin/redis_maintenance ] && /usr/bin/redis_maintenance
