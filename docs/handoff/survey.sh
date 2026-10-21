#!/bin/bash
cd /root/mh1-xbox
cat /root/allmh1.txt | xargs -P4 -I{} sh -c 'VERSION=mhg timeout 300 python3 tools/check.py {} 2>/dev/null | sed "s|^|{} |"' > /root/survey.txt
echo SURVEY_DONE
