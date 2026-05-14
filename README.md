# multiple-network-deauth

It is compiled with `make` on Linux.
**IT DOESN'T WORK ON WINDOWS**
Now it supports TXT databases, which are easier to edit and make!

## Dependencies

- `make`
- `aircrack-ng`
- `iwconfig`

## Usage

Before you do anything, it would not be bad idea to run `touch networks.json`. Then, database file is networks.json.

### Add

Example:

```bash
  sdn add AA:BB:CC:11:22:33 4 networks.jsom
```

Arguments in order:

- bssid
- channel
- database file

### Remove

Example:

```bash
  sdn remove AA:BB:CC:11:22:33 networks.jsom
```

Arguments in order:

- bssid
- channel
- database file

### Takedown

Example:

```bash
  sudo sdn takedown 10 wlan0mon networks.json
```

Arguments in order:

- packet number
- interface
- database file
  **This is how you could put ALL your networks into a DB file**
  1. You run this: `sudo airodump-ng wlan0mon --write scan --output-format csv` **Replace wlan0mon with your network interface**
  2. you run `bash
awk -F',' 'NR>2 && $1 ~ /:/ {  
gsub(/ /, "", $1)
gsub(/ /, "", $4)
print $1, $4
}' scan-01.csv > fb.txt`
  3. Your DB is built, file is `fb.txt`

## Notes

- `add` and `remove` shall only be used with JSON databases.

```

```
