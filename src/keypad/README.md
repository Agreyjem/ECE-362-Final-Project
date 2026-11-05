BEYBLADE RESEARCH SHEET

4 buttons: UP/DOWN/LEFT/RIGHT codes to 2/8/4/6
Player can nagivate using ARROW KEYS or (ABCD) and (12345)

Beyblade basics: 
5 stats per beyblade: atk, def, sta, bur, ss
(attack, defense, stamina, burst resistance, spinsteal)
3 layers: blade, disc, tip

Release:
Can be released at different angles or speeds. 
Entrance angle (spinning around arena to directly into centre)
    ang [0-180], a=1-|ang-90|/90
    0-89 -> right spin (sd=1)
    90 -> head on (sd=1, a=1)
    91-180 -> left spin (sd=-1)
Beginning spin speed (faster is generally better)

Normalize s, a to [0, 1]
As angle increases:
    atk+++    sta---    def--'    bur-''    ss'''
MOD .5+.7a    1.25-.5a  1.15-.3a  1.1-.2a  0+0a

As spin increases:
    atk++'    sta++'    def++'    bur+''    ss++'
MOD .65+.45s  .7+.4s    .8+.3s    .9+.15s   .7+.4s

Diminishing returns on spin? Self ko on max spin (if its too easy to hit max speed)

pscore = .25atk + .2def + .3sta + .15bur
ss=(spinA==spinB) ? 0 : .15ss
ts=pscore+ss
where K is random factor (% randomness?)
p_a_win= 1/(1+exp(-k(ts_a-ts_b)))

type of ko generally: 55% spinout (sta sta), 28% KO (atk def), 17% burst (atk bur)
assuming A win:
a_ko_win=p_ko*exp(0.05(atk_a-def_b))
