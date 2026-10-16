# Hotel Haven — Hotel Room Reservation System

A focused, locally hosted hotel reservation web app built with a C++ OOP backend and a responsive HTML/CSS/JavaScript frontend. Browse rooms, check availability, make a reservation, look up reservations by email, and cancel a confirmed stay.

## Run it on Windows

1. Install a MinGW C++ compiler and make sure `g++` is available on `PATH`.
2. Open this folder in File Explorer.
3. Double-click `run.bat`, or run `run.bat` from a terminal.
4. Open [http://127.0.0.1:8080](http://127.0.0.1:8080) in your browser.
5. Keep the terminal window open while using the app. Press **Ctrl+C** in it to stop the server.

If port 8080 is already in use, run `run.bat 8090` and open `http://127.0.0.1:8090` instead. No database, JavaScript package manager, or third-party C++ library is required.

### Run it from Visual Studio Code

1. In VS Code, choose **File → Open Folder...** and open this project folder.
2. Open the integrated terminal with **Terminal → New Terminal** (or press **Ctrl+`**).
3. Run `.\run.bat`. The script builds the C++ server and starts it in that terminal.
4. Open `http://127.0.0.1:8080` in your browser. Leave the VS Code terminal running while using the app; press **Ctrl+C** there to stop it. Stop an existing server before rebuilding—the running executable cannot be overwritten on Windows.

To run the C++ reservation logic tests, use `test.bat`.

## What the app does

- Lists six sample rooms across Standard, Deluxe, and Suite categories.
- Includes larger, more readable interface text, visible keyboard focus, and an accessible light/dark theme toggle that remembers the selected theme in the browser.
- Uses locally served hotel-room photos from Unsplash: [Classic King](https://images.unsplash.com/photo-1611892440504-42a792e24d32), [Deluxe Garden](https://images.unsplash.com/photo-1616594039964-ae9021a400a0), and [The Corner Suite](https://images.unsplash.com/photo-1590490360182-c33d57733427).
- Features an accessible, auto-advancing hero slideshow for Classic King, Deluxe Garden, and The Corner Suite, with manual controls and reduced-motion support.
- Checks date ranges against confirmed reservations, including stays that cross midnight or month boundaries.
- Validates the number of guests against each room's capacity.
- Calculates the total from each room's nightly rate and the number of nights.
- Assigns a reservation reference, persists bookings in `data\bookings.csv`, and reloads them after a restart.
- Looks up and cancels reservations using the booking email and reference.
- Serves the web UI and its JSON API from the same local C++ server.

Prices are illustrative and shown in INR. The demo does not collect payment. Reservation data stays in the local CSV file. Email lookup is for a classroom demonstration, not authentication for a public production system; do not use real guest data.

## OOP concepts demonstrated

| Concept | Where it appears |
| --- | --- |
| Encapsulation | `Room` and `Booking` keep their state private and expose intentional operations and read-only accessors. |
| Abstraction | `Room` defines the common contract for hotel rooms; `BookingRepository` describes loading and saving without tying the service to CSV. |
| Inheritance | `StandardRoom`, `DeluxeRoom`, and `SuiteRoom` specialize `Room`. |
| Runtime polymorphism | The catalog stores `shared_ptr<Room>` and the server reads each room's virtual category through the base type. |
| Composition and collaboration | `BookingService` coordinates the room catalog, bookings, and repository; each reservation refers to its room and dates. |
| Separation of concerns | HTTP handling, reservation rules, domain objects, persistence, and browser presentation live in distinct files. |

## Project quality checklist

| Goal | Current status |
| --- | --- |
| Responsive layout | Responsive CSS layouts are provided for mobile, tablet, and desktop widths. Check the page at 320 px, 390 px, 768 px, 1024 px, and a wide desktop size when making UI changes. |
| Performance | Room photos are local JPEGs, loaded lazily in room cards, and cached by the browser. The three photos together are about 360 KB. When changing CSS/JS or replacing images, increment their `v` query parameter in `public\index.html` and `public\app.js` because those assets use long-lived browser caching. This small demo has no build pipeline or production load testing; the C++ server handles one request at a time. |
| SEO | The HTML has a page title, description, document language, semantic headings, and social sharing title/description. Search engines cannot index `127.0.0.1`; SEO ranking cannot be assessed unless the app is hosted on a public domain. Before sharing publicly, add a canonical URL and a public absolute sharing image URL. |
| Accessibility | The UI includes form labels, image alternative text, keyboard focus styles, a skip link, a keyboard-operable slideshow with pause/resume, and reduced-motion support. This is a good baseline, not a guarantee of access for every user; test with keyboard navigation, zoom, and a screen reader. |
| UI/UX | The UI has light/dark themes, room photos, booking feedback, and responsive layouts. Validate it with real users and devices rather than treating visual checks as proof of usability. |
| Maintainability | C++ domain, service, persistence, and HTTP layers are separated; browser markup, styling, and interactions are separate files; reservation logic has a dependency-free test executable. |

### Prepare a GitHub portfolio repository

1. Build and test locally with `.\build.bat` and `.\test.bat`.
2. Check that `data\bookings.csv` contains no personal test data before sharing. It is ignored by Git; do not force-add it or the generated `.exe`.
3. In VS Code, open **Source Control** and initialize a repository, or use `git init` in the project terminal.
4. Review **Source Control → Changes** and stage the source, `public` assets, tests, `.gitignore`, and README. Do not stage generated binaries or real guest data.
5. Create a new empty repository on GitHub. Copy its repository URL, then in the terminal run:

   ```powershell
   git add README.md .gitignore build.bat run.bat test.bat include src public tests
   git commit -m "Add Hotel Haven reservation demo"
   git branch -M main
   git remote add origin YOUR_GITHUB_REPOSITORY_URL
   git push -u origin main
   ```

   Replace `YOUR_GITHUB_REPOSITORY_URL` with the URL GitHub gives you. If you already added a remote, inspect it with `git remote -v` before adding another.

### Add screenshots to the repository

The laptop/PC and mobile screenshots are already saved in the `ScreenShots` folder. These embeds link to each image file:

#### Laptop / PC view

![Hotel Haven laptop/PC header and slideshow](./ScreenShots/LAPTOP_PC%20VIEW/HEADER_AND_SLIDESHOW.png)

![Hotel Haven laptop/PC rooms 101, 102, and 103](./ScreenShots/LAPTOP_PC%20VIEW/ROOM_NO_101_102_103.png)

![Hotel Haven laptop/PC rooms 201, 202, and 301](./ScreenShots/LAPTOP_PC%20VIEW/ROOM_NO_201_202_301.png)

![Hotel Haven laptop/PC room-specific dates](./ScreenShots/LAPTOP_PC%20VIEW/SPECIFIC_ROOM_WISE_DATE.png)

![Hotel Haven laptop/PC reservation email lookup](./ScreenShots/LAPTOP_PC%20VIEW/DEMO_EMAIL.png)

![Hotel Haven laptop/PC footer](./ScreenShots/LAPTOP_PC%20VIEW/FOOTER.png)

#### Mobile view

![Hotel Haven mobile header](./ScreenShots/MOBILE%20VIEW/HEADER.png)

![Hotel Haven mobile slideshow and room search](./ScreenShots/MOBILE%20VIEW/SLIDESHOW_AND_SEARCH_ROOMS.png)

![Hotel Haven mobile rooms 101 and 102](./ScreenShots/MOBILE%20VIEW/ROOM_101_102.png)

![Hotel Haven mobile room 103](./ScreenShots/MOBILE%20VIEW/ROOM_103.png)

![Hotel Haven mobile rooms 201 and 202](./ScreenShots/MOBILE%20VIEW/ROOM_NO_201_202.png)

![Hotel Haven mobile room 301](./ScreenShots/MOBILE%20VIEW/ROOM_301.png)

![Hotel Haven mobile reservation email lookup and footer](./ScreenShots/MOBILE%20VIEW/DEMO_EMAIL_%26_FOOTER.png)

Check each screenshot for real guest details before publishing it.

### Resume entry example

> **Hotel Haven — C++ Hotel Reservation Web App**  
> Built a locally hosted hotel reservation demo with a C++ OOP backend and responsive browser UI. Implemented date-based availability, guest-capacity validation, booking cancellation, CSV persistence, and automated service tests.

Describe it as a local demo unless you actually deploy and operate it. The email-based booking lookup is not secure user authentication.

## Project structure

```text
include/     Domain, repository, service, and server interfaces
src/         C++ implementations and application entry point
public/      Browser UI assets
tests/       Dependency-free C++ reservation service tests
data/        Created at runtime; CSV reservations are stored here
```

## Local JSON API

- `GET /api/health` — server status.
- `GET /api/rooms?checkIn=YYYY-MM-DD&checkOut=YYYY-MM-DD` — rooms available for the requested stay.
- `POST /api/bookings` — create a reservation using URL-encoded `roomId`, `guestCount`, `guestName`, `email`, `checkIn`, and `checkOut` fields.
- `GET /api/bookings?email=guest@example.com` — find reservations for an email address.
- `POST /api/bookings/cancel` — cancel using URL-encoded `bookingId` and `email` fields.

The server binds to `127.0.0.1` only. This intentionally small project is a learning/demo application, not a production hotel system: it has no user accounts, payment processing, database concurrency, or production-grade HTTP/TLS stack.
