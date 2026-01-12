# The boardgame of all time

## Inspirations:
This game has two distinct inspirations:

Highschool study hours were pretty boring. To compensate, I folded sheets of my algebra II graph paper and began drawing the typical battleship alphanumeric grid. After a few classmates came to gawk at my progress, I acquired one intrigued classmate after the other, until my hands were exhausted and nearly half the room was playing battleship. I fortunately had the rules of play handy, and we all had a great time. I wanted this simple boardgame to have the same homebrew appeal, but taking advantage of automation and an ulterior vision where possible.

The ulterior vision came out of the synthesis of my Digital Logic Design classes at Uni, an events organizer who we will name as Nathan, and a fellow boardgame enthusiast we will name Andrew. Previously, Nathan had hosted parties to enjoy such great titles as *Colonizing Mars*, *Tsuro*, and *Cockroach Poker*. As much as a fan of novelty as I perceived him to be, I made it my prerogative for a time to suggest classic titles wherever I could. There was a great amount of back log as Nathan drew suggestions from both the typical party-goers and myself. One such item in the back of the queue was a family favorite of mine: *Rummy*. *Rummy* is a very unassuming game, but it scales with the skill of players well and occassionally you can drive your wits to a win that doesn't feel like the game just simply took you there.

Andrew also likes *Rummy* (albeit not to the same extent as I), and was excited at our combined contributions to the gaming scene as coordinated by Nathan. We proposed such titles as *Scrabble* and *Rummikub*, but ultimately we wanted something more.

Then one day I woke up and I envisioned a hybrid between *Rummy* and *Battleship*. After tossing ideas around, Andrew and I came up with a codified ruleset. This repository contains the result.

# Description:
## How cards are incorporated
The game is to be played with two standard 52-card decks (4 suits by 13 ranks). The order of ranks is A23456789JQK. Each face card has an obvious numeric weight (i.e. K = 13). Initialization is simple and straightforwardly puts the game master in charge of a suite of player customization. First the game master must combine four randomly dispersed tiles given the arrow keys. Select a suit to select one of the four tiles. After bringing the four tiles together into a 2x2 arrangement, the game master must press 's' to save player or 'r' to reset player. If 's' is selected, a player color picker will appear next. After selecting the appropriate color press 's' to continue. Finally, the game master will move the fully formed-player to a suitable board location. If the game master wishes to start the game now, he may press 'c', otherwise to build another player he must press 's'.

The game master, upon game start, will have the responsibility of recording all player card forfeitures. If this is not done quickly, the game can get pretty boring while players wait for their played cards to be recorded. The developer takes this to be a *major fault of the game* and not one of any game master.

Each player should be dealt 7 cards. A maximum of 4 cards may be played at any player's turn, but only one may be retrieved from the draw pile each turn. Played cards are fully discarded and are not reused (again another fault of the game). Cards with similar suit may not be played concurrently. A player may choose to not play any cards during a turn.

Having played a card garauntees that the relevant player's raft will be moved in a manner hereafter described. The card's suit will determine which cannon from which a cannonball will be blown. The card's rank will determine the cannonball's distance *AND* a reverse movement of the raft (think recoil). The raft will reverse (recoil) a distance 1/2 the distance of the cannonball rounded down. An example: a player plays a *9 of Spades* and thus blows a cannonball 9 units (from their Spade cannon) in front of them. The player also recoils 4 (floor(9 / 2)) units.

## Cannonball aftermath
A player which has been hit by a cannonball, loses a (whole) cannon on the relevant tile on which they were struck. This cannon is no longer usable in future play, and in some cases renders a particular card type in their deck essentially inert (another fault of the game). 

## Regarding collisions
Collisions offer great creative potential to the player. Regaining cannons is possible, if a player calculates his moves correctly. A number of potential collision configurations are possible. In configuration 1, If a full-raft overlap occurs, the player with priority (see game code) moves one more tile in the direction of their recoil. This places them in configuration 2. If no additional recoil is on the player, a random direction (perpendicular to the previous recoil) is chosen and the raft is moved that direction one unit. The goal is the enter all rafts into configuration 3 in which rafts collide only on one tile. If a pair of rafts are in configuration 2, we move them to configuration 3.

In configuration 3, the collided tiles do one of three things:
  + Exchange an active cannon on player 1 for an inactive cannon on player 2 (giving player 2 a new cannon)
  + Exchange an active cannon on player 2 for an inactive cannon on player 1 (giving player 1 a new cannon)
  + Destroy both cannons, provided both players have an active cannon on the tile where the collision happened

The third undesirable option is what makes collisions risky, yet palpable.

... and I think that's everything ...

## Installation (approximately):
Warning: You must have a slightly modified version of [Some Simple Wrappers](https://github.com/xleph/ssw) (ssw) installed system wide for the following:

`(g)cc queue.c boardgammon.c -o boardgammon -lssw`

to compile and link properly.

## Prerequisites:

I never intended for this game to leave my Surface Go + Arzopa setup, but in the case that you wanted to endure a game I suggest using two displays: a main 1280x720 display and a 1080x720 display.
