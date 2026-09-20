# Installing Anaconda XB 360 on Xbox 360

## Requirements

- Modded Xbox 360 (RGH/JTAG, or a working software exploit)
- Aurora dashboard installed
- FTP access or a USB drive
- The `script/` folder contents from this repo

## Steps

1. Connect to your console via FTP, or plug in a USB drive.

2. Copy the `script` folder contents to:

   Hdd:\Aurora\User\Scripts\Utility\AnacondaXB360\

   You should end up with:

   Hdd:\Aurora\User\Scripts\Utility\AnacondaXB360\AnacondaXB360.lua
   Hdd:\Aurora\User\Scripts\Utility\AnacondaXB360\anaconda.cfg

3. On the console, open Aurora.

4. Press the Back button to open the Aurora menu.

5. Go to Scripts > Utility > AnacondaXB360.

6. The store loads your repository and shows the category menu.

## Changing the repo URL

Inside the app, choose **Settings** and enter a new URL.
It is saved to `Hdd:\Anaconda\anaconda.cfg`.

## Folder layout on the console

| Path | Purpose |
|---|---|
| `Hdd:\Anaconda\anaconda.cfg` | Saved settings |
| `Hdd:\Anaconda\anaconda.log` | Debug log |
| `Hdd:\Anaconda\cache\` | Temporary downloads |
| `Hdd:\Aurora\User\Scripts\Utility\AnacondaXB360\` | The app itself |

## Updating

Replace `AnacondaXB360.lua` with the newer version from GitHub.
Your `anaconda.cfg` stays as-is, so your settings are preserved.

## Troubleshooting

**The store says "Could not reach repository":**

- Check that your console has internet access.
- Open the repo URL in a browser on your PC to confirm it loads.
- Make sure the URL points to the raw file, not the GitHub HTML page.

**Downloads fail partway through:**

- Large `.7z` files should be hosted in GitHub Releases, not in the repo itself.
- Check `Hdd:\Anaconda\anaconda.log` for the exact error.
