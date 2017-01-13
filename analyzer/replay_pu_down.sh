#!/usr/bin/env bash

cp mis_cards/replay_nominal.yml mis_cards/config_replay/replay_pu_down.yml
sed -i 's/jjbtag\*pu\*event_weight/jjbtag\*pu_down\*event_weight/g' mis_cards/config_replay/replay_pu_down.yml
sed -i s/nominal/pu_down/g mis_cards/config_replay/replay_pu_down.yml
python/replay.py mis_cards/config_replay/replay_pu_down.yml
