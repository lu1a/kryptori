## WIP! Make some PRs if you want this to happen faster!

# kryptori

tori.fi, ebay, etc., but for exchanging cryptocurrencies and fiat P2P.

LocalMonero is down, LocalBitcoins is down, somebody needs to pick up the slack.

This codebase aims to create a NOT FOR PROFIT platform for users to exchange crypto with random people online 
through public advertisements. This platform will not initially be an escrow service.

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
- users who post an ad must be able to come back and manage the advertisements they make.
- web server: full functionality with HTML only. (With enhanced functionality with JS enabled?)

Here's how I imagine the process to go:

1. A user posts an ad, and recieves a token via email where they can manage said ad.
2. An interested party clicks that they're interested, and is prompted to give a reply and their email address to the ad poster.
3. This site sends that interested party reply+caller email to the ad poster user via the poster's email.
4. The poster gets in touch with the interested party and sorts everything out with them via email between themselves.
5. Transaction occurs or doesn't, between themselves. The poster takes down or edits the ad via their token they got sent to their email originally.

## Run the application

`nix develop` then `make run` or `uvicorn main:app --reload`

## Models

### Advertisement

| Field                 | Description                                                                                                                            |
|-----------------------|----------------------------------------------------------------------------------------------------------------------------------------|
| **id**                |                                                                                                                                        |
| **created_at**        |                                                                                                                                        |
| **updated_at**        |                                                                                                                                        |
| **title**             |                                                                                                                                        |
| **description**       |                                                                                                                                        |
| **active**            | Whether the ad has been activated yet by user, proving they're alive by navigating to their ad management page via unique token param. |
| **owner_email**       | To send to only, never to show on the site.                                                                                            |
| **owner_token**       | So that there's a way for the OP to manage the ad after posting.                                                                       |

