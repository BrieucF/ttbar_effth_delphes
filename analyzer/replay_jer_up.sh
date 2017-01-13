#!/usr/bin/env bash

# Jet energy resolution up
cp mis_cards/replay_nominal.yml mis_cards/config_replay/replay_jer_up.yml
sed -i s/flatTrees_with_weight_v2/jerup_weightProd_v2/g mis_cards/config_replay/replay_jer_up.yml
sed -i s/nominal/jer_up/g mis_cards/config_replay/replay_jer_up.yml
python/replay.py mis_cards/config_replay/replay_jer_up.yml
