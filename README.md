# Spark, the dual-themed static blog generator
## Overview
I built Spark for my website (https://thinkingincode.ninja) and am making it publicly available.
Basically, it's a static blog site generator, where all content is defined in easy to work with folders and files. Static, in this context, means that only HTML (and RSS) files are generated, there's no PHP or anything that is executed dynamically when pages are loaded. It was created to generate a site where only one HTTP request is needed per-page, the request to get the HTML file (ie CSS is embedded in a `<style>` tag). 

The idea is that each post belongs to a 'series'; this is different from, and in addition to, tags, of which each post needs at least one. Maybe one series is about cooking recipes, another about project ideas, and yet another about interesting things you learn each day. Each series has a listing page of all published posts in that series. 

The name of it is **Spark**, because it's a *static* blog generator.
Key features:
- Site content is defined using folders and files (the first iteration of the generator was a series of shell scripts, so having separate files for things like the title or description worked quite well)
  - All files are plain text
- Pages and posts are written in HTML, and can contain arbitrary HTML
- Pages are wrapped up in a header and footer that give the entire site a uniform appearance
- Generates an RSS feed file
- Posts can be hidden until after a specific publish date/time
- CSS is added in a `<style>` element, not linked to as a separate file
- Posts marked as having code will have CSS be added for syntax highlighted `<span>`'s (note, the `<span>`'s are not calculated, you have to manually write them out in the post HTML file)
- Generates two versions of the site, one with a dark theme, one with a light theme; all generated pages contain links to the alternate theme page
  - Designed to be used with separate domain names for each theme, though I think it could work with a single domain name
- Output files won't be written if they wouldn't be changed
- Fairly unopinionated about the appearance and structure of the site it generates. It does dictate the page header, and post footer, and maybe a couple of other small things, but (with code changes) this can be changed.
- Home page is generated with 'new and updated posts'
- All links are generated with https, not http
- Assumes that the web server is configured to find .html files even if the extension isn't in the URL (ie links don't have the .html file extension)
- Extensive error checking and handling is done; Spark is very unlikely to outright crash. If there's a problem generating your site, the combination of error messages it outputs should be enough to figure out what you did wrong (or, if it's a problem with Spark itself, what line it failed on).
  - Nothing will be generated if you have invalid files, though Spark assumes that the user is providing valid HTML and such.
- Posts have footers generated that contain the author, timestamps, tags, and series
- Extremely fast

I do intend on eventually adding in some scripts to make it so that all you have to do is have a blank CentOS server and run a couple of scripts to get a blog site up and running.

# Using Spark
## Site file structure
There need to be the following folders. They will be described in more detail below.
- `/misc_pages/`: Contains pages like the home page, about page, etc. There's a folder per page.
- `/themes/`: Contains two folders, `dark` and `bright`; each contains CSS files.
- `/series/`: Contains a folder for each series.
- `/posts/`: Contains a folder for each post.
- `/components/`: Contains some key HTML fragments.

### `/misc_pages/`
There is a folder per file; every file will be generated, at this time there is no flag to toggle the generation of a `misc_page`.

The information for the index file is required.

Each `misc_page` folder contains the following files:
- `filename`: What URL/file path the file should have. At this point, Spark does not create intermediary folders.
- `description`: A brief description; this goes in a `<meta>` tag in the HTML `<head>`. Must not contain double quotes or any unescaped HTML characters (such as greater than/less than signs).
- `title`: The page title; this goes in the HTML `<title>` tag. Must not contain unescaped HTML characters.
- `content.html`: The page content.

Note, the index page is treated differently from every other `misc_page`; it has additional content added to it, the 'new and updated posts' list.

### `/themes/`
There are two folders, `dark` and `bright`. Each presently has the following CSS files:
- `main.css`: The main CSS file that is included on every page.
- `syntax-highlighting.css`: A CSS file that's included in each post that has the `has-code` flag set.

### `/components/`
There are presently three component files:
- `header.html`: The opening `<html>` and `<head>` tags, as well as any other things you want to include in the `<head>`, such as a Flattr ID tag.
- `footer.html`: The site navigation footer.
- `trailer.html`: The closing `</html>` tag, and anything else you want to put in at the end of each file.

### `/series/`
There's a folder for each series. Presently, every series will be generated, there is no flag to toggle the generation of a series.

The name of the series folder matters; this is both the ID of the series, and what the filename/URL will be for the series landing page. For example, a series described in the folder `my-cooking-recipes` will be accessible as `myblog.example.com/series/my-cooking-recipes`.

Each `series` folder has the following files:
- `order`: A file containing a number; all series are sorted on the `order` value, in ascending order.
- `short-description`: A brief description; this goes in a `<meta>` tag in the HTML `<head>`; must not contain double quotes or unescaped HTML characters.
- `title`: The page title; this goes in the HTML `<title>` tag; must not contain unescaped HTML characters.
- `landing-desc.html`: A chunk of HTML to be used on the series landing page (which shows all posts in the series) as well as on the page that shows all series. Should give a nice description of what the series is all about.

### `/posts/`
There's a folder for each post. Unlike the other data folders, not all files are required for posts.

The name of the post folder matters; this is both the ID of the post, and what the filename/URL will be for the post. For example, a post described in the folder `my-first-post` will be accessible as `myblog.example.com/posts/my-first-post`.

Required files:
- `title`: The title of the post. Will be automatically added in an `<h1>` header on the page, as well as to the `<title>` tag in the `<head>`. Can't contain double quotes or any unescaped HTML characters.
- `short-description`: A brief description; this goes in a `<meta>` tag in the HTML `<head>`; must not contain double quotes or unescaped HTML characters.
- `long-description`: A description of the post that's a few sentences long. This description will be used on any page where the post is listed, as well as on the RSS feed. Can't contain unescaped HTML characters.
- `series`: The ID (folder name) of the series this post belongs to. Must be a valid series ID.
- `author`: Who wrote the post. Will be added in a `<meta>` tag in the `<head>`.
- `written-date`: Not actually a date, it's a date/time. When you are wanting to say that you wrote the post. The string will be displayed on the site, so make it human readable. Will be used in the RSS feed as the publication date/time. Must be any time string that can be parsed by the Unix `date` command; I personally write it in the format `9:00AM CDT 6/3/2018`. Do include a timezone, or be consistent in not including one, otherwise you might get inconsistent results.
- `tags`: A comma-separated list of tags that you want this post to be linked to. Also added in the `keywords` `<meta>` tag. Can't contain double quotes, spaces or unescaped HTML entities.
- `content.html`: The HTML file for the post. 

Required files if you want the post to show up:
- `generate-post`: It doesn't matter what this file has as contents, all that matters is whether or not it exists. If it exists, it will cause Spark to create the HTML page for the post. If only this flag is present, then the post won't be "published", which means that the post won't be linked to from any other pages. If this flag is missing, Spark will completely ignore the post, only printing out a message saying that it's skipping the post.
- `publish-when-ready`: Will cause Spark to publish the post (link it in from other pages) if it exists. The 'when-ready' part refers to the `publish-after` file described below.

Optional files:
- `has-code`: If present, will cause Spark to include the syntax-highlighting.css file contents.
- `publish-after`: A date/time string; Spark will not publish this post (if the `publish-when-ready` flag is present) until this time.
- `updated-at`: A date/time string; newly updated posts will appear on the home page.
- `suggested-prev-reading`: A file, with one valid post ID (post folder name) per line. Each post listed will be included (with links) in a blurb at the top of the post titled "Suggested previous reading". Posts that aren't to be published will be skipped.
- `suggested-next-reading`: A file, with one valid post ID (post folder name) per line. Each post listed will be included (with links) in a blurb at the bottom of the post titled "Suggested next reading". Posts that aren't to be published will be skipped.

## Site configuration file
In order for Spark to work, you need to have a configuration file for each site. I designed it so that I could have preview and live versions of my site, each with their own configuration, but both backed by the same git repository.

The name of the file doesn't matter, you'll be passing it in as a parameter to Spark. Whatever you name it, if you do decide to manage the site contents via a git repository, make sure you add the filename to your `.gitignore` file.

The format of the configuration file is this: One variable per line; `VARNAME=value`, no spaces around the equals sign. No quotes around strings.

The following configuration options exist right now, though some of these will change, and others will be added in. All are required.
- `BRIGHT_HOST`: The hostname for the bright site.
- `DARK_HOST`: The hostname for the dark site.
- `BRIGHT_NAME`: The text that will appear when you're on a dark page and looking at the link that will take you to the bright-themed version of the same page. You'll probably want to keep this as 'Bright'.
- `DARK_NAME`: The text that will appear when you're on a bright page and looking at the link that will take you to the dark-themed version of the same page. You'll probably want to keep this as 'Dark'.
- `HTML_BASE_DIR`: The root HTML directory (with no trailing slash). This directory will be populated with a 'dark' and 'bright' folder. Make sure whoever Spark runs as has the appropriate rights to do so.
- `CODE_BASE_DIR`: The name of this variable will change in the future. This is the base site content directory (with no trailing slash), the directory that has all of the folders and files detailed above.
- `SITE_GROUP`: Presently unused, this is the name of the group that all created files/folders will be owned by (note, this doesn't happen yet, I haven't gotten around to doing this yet).

## How to compile Spark
This assumes that you have a `gcc` compiler.

Change to the directory that has Spark (assuming you did a `git clone` or something like that).
Run `./compile`; this will create a folder `bin/` in this directory, and populate it with the `spark` executable.

## How to run Spark
Create the folder/file structures defined above (I'll eventually add in an example site to this repository).
Create a configuration file as defined above.

Run the `spark` executable compiled above, passing it `--config /path/to/your/site/config/file` (putting in your site configuration file as appropriate).

# Issues and bugs
There are 2 places that reference my site specifically (the site header and the RSS description). I will be fixing this in the very near future.

When checking the existing RSS file against the newly generated one, to see if it needs to be written out, I don't do proper bounds checking. This will lead to a crash only if the existing RSS file is malformed.

I need to make an example site to include with this repository, so that you can actually see the folder/file structure.

Spark assumes that the user is going to write content and files that will eventually lead to pages being generated that are valid HTML. This isn't really an issue, but I'm putting it out there. I think it'd be too time-consuming to have Spark validate that every single string is correct, and that your pages have proper HTML and all that. I may eventually put something together that'll do that kind of validation, but it'll never be something that's done every time a site is generated.

Spark only cleans up (deletes) old tag files, I don't yet have it deleting series/posts that no longer exist.

Various other bugs and things exist. I am going to be refactoring various pieces, to make it simpler.
