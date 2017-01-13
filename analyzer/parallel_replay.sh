#!/usr/bin/env bash
#function run_replay {
    echo $1 # $1 will be replaced by $2
    echo $2
    cp mis_cards/replay_nominal.yml mis_cards/config_replay/replay_$2.yml
    sed -i s/$1/$2/g mis_cards/config_replay/replay_$2.yml
    sed -i s/nominal/$2/g mis_cards/config_replay/replay_$2.yml
    python/replay.py mis_cards/config_replay/replay_$2.yml
#}
#run_replay jjbtag jjbtag_up
#run_replay jjbtag jjbtag_down
#
#run_replay llidiso llidiso_elidisoup
#run_replay llidiso llidiso_elidisodown
#
#run_replay llidiso llidiso_muidup
#run_replay llidiso llidiso_muiddown
#
#run_replay llidiso llidiso_muisoup
#run_replay llidiso llidiso_muisodown
#
