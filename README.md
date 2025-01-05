## WIP! Make some PRs if you want this to happen faster!

# kryptori

tori.fi, ebay, etc., but for exchanging cryptocurrencies and fiat P2P.

LocalMonero is down, LocalBitcoins is down, somebody needs to pick up the slack.

This codebase aims to create a NOT FOR PROFIT platform for users to buy crypto from random people online,
through advertisements of exchange. This platform will not initially be an escrow service,
only advertisement.

For example, say I want to get more Monero. It's annoying to go through a third-party
cryptocurrency to go from fiat currency -> Bitcoin (for ex.) -> Monero, all the while losing
bits of my money along the way. I would rather find someone who's willing to just do an exchange
of their Monero for my cash, either by mail or in person (yes, I'm brave).

The ads on this platform will look like this:
- I have 180EUR, I want 1XMR (in person, Uusimaa, Finland)
- I have 0.003BTC, I want 290EUR (cash by mail, Europe)
- I have the latest thinkpad with xyz specs, I want 5XMR (in person or by mail, Stockholm, Sweden)
etc.

Then the interested parties will iron out the details of their exchange via chat.
That's the make-or-break of this platform, relying on others to conduct themselves honourably
and honestly - but it's up to each willing adult to use their best judgement.

What do I get out of it, you ask? First of all I literally myself want to exchange currencies, second of all
I guess I'll chuck a donation link on the site somewhere..

## The requirements/constraints of this codebase

- no accounts, no logging of user data, full stop.
- must encourage users' self-sufficiency and liberty. No hosted Bitcoin wallets, but good instructions on how to make one's own.
- users must be able to come back and edit/reply to advertisements they make.
- users who answer advertisements must be able to return after they initially register as interested.
- web server: full functionality with HTML only, but enhanced functionality with JS enabled.

## Run the application

`cc main.c deps/mongoose.c deps/sqlite3.h`

## Models

### Advertisement

| Field                 | Description                                                                      |
|-----------------------|----------------------------------------------------------------------------------|
| **id**                |                                                                                  |
| **created_at**        |                                                                                  |
| **updated_at**        |                                                                                  |
| **title**             |                                                                                  |
| **description**       |                                                                                  |
| **is_standing_order** | Determines whether the ad should be deleted when the exchange is marked as over. |
| **owner_token**       | *(foreign key)*                                                                  |
| **chats**             | *(foreign key)*                                                                  |

### User

| Field                  | Description     |
|------------------------|-----------------|
| **token**              |                 |
| **last_interacted_at** |                 |
| **chats**              | *(foreign key)* |

### Chat

| Field                | Description     |
|----------------------|-----------------|
| **id**               |                 |
| **advertisement_id** | *(foreign key)* |
| **owner_token**      | *(foreign key)* |
| **seeker_token**     | *(foreign key)* |
| **messages**         | *(foreign key)* |

### Message

| Field                | Description     |
|----------------------|-----------------|
| **id**               |                 |
| **sent_at**          |                 |
| **read_at**          |                 |
| **text**             |                 |
| **sender_token**     | *(foreign key)* |
| **chat_id**          | *(foreign key)* |

