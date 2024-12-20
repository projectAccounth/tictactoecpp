# tictactoecpp

So, what is this? This is a simple tic-tac-toe library, with a simple move validation system, and is made in C++.

What's so special about this?

This supports larger boards like 4x4, 5x5, ..., up to NxN, with N being any positive integer larger than 3. You can also adjust the match length for your game.

This also comes a simple engine that you can use to analyze the position and retrieve the best moves. Note that this engine is pretty bad and you can use this for testing purpose.

This board uses 1-indexing for making moves. For example, on this 3x3 board:

```
| | | |
-------
| | | |
-------
| | | |
```

Making a move on (2, 2) will place X (assuming that X takes the first move) at the center of the board:
```
| | | |
-------
| |X| |
-------
| | | |
```

You can also get the result of your current game using a method in the class. The format of the result is similiar to chess's PGN notation - I took inspiration from this!

I will document this library soon. I don't have the motivation to read the mess I've created.
