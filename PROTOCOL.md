# Client <-> Server Protocol

This document is the written contract for every WebSocket message exchanged
between a client and the server. It defines message *shape* only - no
send/receive wiring, no matchmaking/persistence logic, and no connection to
`GameEngine` exist yet. Those are deliberately deferred to later steps; this
step only fixes the contract they will be built against.

## Envelope

Every message, in both directions, is wrapped in the same envelope:

```json
{
  "type": "<message_name>",
  "payload": { ... }
}
```

- `type` (string) - one of the message names listed below, exactly as
  written (snake_case).
- `payload` (object) - the message's own fields, as documented per message.
  An empty-payload message still sends `"payload": {}`, not an omitted key.

---

## Client -> Server

### `login`
Authenticate a user before any room/matchmaking action.

| field      | type   |
|------------|--------|
| `username` | string |
| `password` | string |

```json
{"type": "login", "payload": {"username": "alice", "password": "hunter2"}}
```

### `register`
Create a new account with the given email/password. Does not log the new
account in - a separate `login` message is still required afterward.

| field      | type   |
|------------|--------|
| `email`    | string |
| `password` | string |

```json
{"type": "register", "payload": {"email": "alice@example.com", "password": "hunter2"}}
```

### `create_room`
Create a new named room and join it as its first player (white).

| field       | type   |
|-------------|--------|
| `room_name` | string |

```json
{"type": "create_room", "payload": {"room_name": "alices-room"}}
```

### `join_room`
Join an existing room by its exact name. Second player to join becomes
black; anyone after that becomes a spectator.

| field       | type   |
|-------------|--------|
| `room_name` | string |

```json
{"type": "join_room", "payload": {"room_name": "alices-room"}}
```

### `quick_play`
Request ELO-based matchmaking using the already-authenticated player's
stored rating (server looks up the rating - the client sends no rating of
its own). Empty payload.

```json
{"type": "quick_play", "payload": {}}
```

### `click`
Forward a single board click's pixel coordinates. Only valid once the
client is inside an active game (as a player); the server maps this
straight onto the existing `Controller::click(state, x, y)` semantics.

| field | type |
|-------|------|
| `x`   | int  |
| `y`   | int  |

```json
{"type": "click", "payload": {"x": 320, "y": 180}}
```

### `leave`
Voluntarily leave the current room/game, whether joined as a player or a
spectator. Empty payload.

```json
{"type": "leave", "payload": {}}
```

---

## Server -> Client

### `login_result`
Reports whether a `login` attempt succeeded.

| field     | type              | notes                                  |
|-----------|-------------------|-----------------------------------------|
| `success` | bool              |                                          |
| `reason`  | string (optional) | present only when `success` is `false`  |
| `rating`  | int (optional)    | present only when `success` is `true`   |

```json
{"type": "login_result", "payload": {"success": true, "rating": 1200}}
```
```json
{"type": "login_result", "payload": {"success": false, "reason": "invalid_credentials"}}
```

### `register_result`
Reports whether a `register` attempt succeeded. Unlike `login_result`,
`reason` is always present (an empty string on success) rather than
optional, since there's no second field (like `rating`) to distinguish the
two cases by presence.

| field     | type   | notes                                   |
|-----------|--------|-------------------------------------------|
| `success` | bool   |                                            |
| `reason`  | string | empty on success; e.g. `"email_taken"` on failure |

```json
{"type": "register_result", "payload": {"success": true, "reason": ""}}
```
```json
{"type": "register_result", "payload": {"success": false, "reason": "email_taken"}}
```

### `room_joined`
Confirms the client has joined a room (via `create_room`, `join_room`, or a
successful `quick_play`) and states its assigned role.

| field     | type                                   |
|-----------|----------------------------------------|
| `room_id` | string                                  |
| `role`    | `"white"` \| `"black"` \| `"spectator"` |

```json
{"type": "room_joined", "payload": {"room_id": "alices-room", "role": "black"}}
```

### `matchmaking_result`
Reports the outcome of a `quick_play` request.

| field     | type              | notes                                       |
|-----------|-------------------|----------------------------------------------|
| `success` | bool              |                                                |
| `room_id` | string (optional) | present only when `success` is `true`         |
| `reason`  | string (optional) | present only when `success` is `false`, e.g. `"no_opponent_found"` |

```json
{"type": "matchmaking_result", "payload": {"success": true, "room_id": "room_42"}}
```
```json
{"type": "matchmaking_result", "payload": {"success": false, "reason": "no_opponent_found"}}
```

### `snapshot`
The current game state, mirroring the existing `GameSnapshot`/`PieceSnapshot`
fields verbatim. Sent to every client in a room (players and spectators
alike; spectators are read-only) periodically or on change.

| field                    | type                                       |
|--------------------------|---------------------------------------------|
| `rows`                   | int                                           |
| `cols`                   | int                                           |
| `pieces`                 | array of piece objects (see below)            |
| `pieces[].pieceCode`     | string, e.g. `"RW"`, `"BB"`                   |
| `pieces[].pixelX`        | int                                            |
| `pieces[].pixelY`        | int                                            |
| `pieces[].animState`     | `"idle"` \| `"move"`                          |
| `pieces[].frameIndex`    | int, 1-based                                   |
| `selectedCell`           | `{row: int, col: int}` or `null`               |
| `gameOver`               | bool                                            |

```json
{
  "type": "snapshot",
  "payload": {
    "rows": 8,
    "cols": 8,
    "pieces": [
      {"pieceCode": "RW", "pixelX": 0, "pixelY": 0, "animState": "idle", "frameIndex": 1}
    ],
    "selectedCell": null,
    "gameOver": false
  }
}
```

### `opponent_disconnected`
Notifies the remaining participant(s) that the opponent disconnected and a
countdown to an automatic loss (for the disconnecting player) has begun.

| field              | type |
|--------------------|------|
| `countdown_seconds`| int  |

```json
{"type": "opponent_disconnected", "payload": {"countdown_seconds": 30}}
```

### `game_over_result`
Announces the end of a game and why.

| field    | type                                  |
|----------|-----------------------------------------|
| `winner` | `"white"` \| `"black"` \| `null` (draw) |
| `reason` | string, e.g. `"checkmate"`, `"disconnect_timeout"`, `"draw"` |

```json
{"type": "game_over_result", "payload": {"winner": "white", "reason": "checkmate"}}
```

### `error`
Generic error report for malformed requests or invalid actions (e.g. a
`click` sent while not in a game, or `join_room` for a room that doesn't
exist).

| field     | type   |
|-----------|--------|
| `message` | string |

```json
{"type": "error", "payload": {"message": "Room 'alices-room' not found"}}
```
