#!/bin/sh
#-----------------------------------------------------------------------------
#
#  Invalid small "island" with coastline ring not closed.
#
#-----------------------------------------------------------------------------

. $1/test/init.sh

set -x

#-----------------------------------------------------------------------------

$OSMC --verbose --overwrite --output-database=$DB --output-rings $DATA >$LOG 2>&1
RC=$?
set -e

test $RC -eq 2

grep 'There are 2 nodes where the coastline is not closed.' $LOG
grep 'Closed 1 rings. This left 0 nodes where the coastline could not be closed.' $LOG
grep '^There were 0 warnings.$' $LOG
grep '^There were 1 errors.$' $LOG

check_count land_polygons 1;
check_count rings 1;
check_count error_points 2;
check_count error_lines 1;

echo "SELECT AsText(geometry) FROM land_polygons;" | $SQL \
    | grep -F 'POLYGON((1.01 1.01, 1.01 1.04, 1.04 1.04, 1.04 1.01, 1.01 1.01))'

echo "SELECT AsText(geometry) FROM error_lines;" | $SQL \
    | grep -F 'LINESTRING(1.01 1.04, 1.01 1.01)'

#-----------------------------------------------------------------------------

set +e

$OSMC --verbose --overwrite --output-database=$DB --output-rings -c 0 $DATA >$LOG 2>&1
RC=$?
set -e

test $RC -eq 2

grep 'There are 2 nodes where the coastline is not closed.' $LOG
grep 'No polygons created!' $LOG
grep '^There were 1 warnings.$' $LOG
grep '^There were 1 errors.$' $LOG

check_count land_polygons 0;
check_count rings 0;
check_count error_points 2;
check_count error_lines 1;

echo "SELECT AsText(geometry), osm_id, error FROM error_lines;" | $SQL \
    | grep -F 'LINESTRING(1.01 1.04, 1.04 1.04, 1.04 1.01, 1.01 1.01)|200|not_closed'

#-----------------------------------------------------------------------------
