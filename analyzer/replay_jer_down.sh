#!/usr/bin/env bash

# Jet energy resolution down
cp mis_cards/replay_nominal.yml mis_cards/config_replay/replay_jer_down.yml
sed -i s/flatTrees_with_weight_v2/jerdown_weightProd_v2/g mis_cards/config_replay/replay_jer_down.yml
sed -i s/nominal/jer_down/g mis_cards/config_replay/replay_jer_down.yml
python/replay.py mis_cards/config_replay/replay_jer_down.yml
