#!/usr/bin/env bash

cp mis_cards/replay_nominal.yml mis_cards/config_replay/replay_pu_up.yml
sed -i 's/jjbtag\*pu\*event_weight/jjbtag\*pu_up\*event_weight/g' mis_cards/config_replay/replay_pu_up.yml
sed -i s/nominal/pu_up/g mis_cards/config_replay/replay_pu_up.yml
python/replay.py mis_cards/config_replay/replay_pu_up.yml
