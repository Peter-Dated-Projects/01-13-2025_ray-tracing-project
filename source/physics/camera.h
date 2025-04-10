

#ifndef camera_h
#define camera_h

#include "hittable.h"
#include "hittable_list.h"
#include "material.h"

#include <atomic>
#include <thread>
#include <vector>

#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <mutex>
#include <fstream>
#include <sstream>
#include <chrono>


class camera {
private:
    // Private Camera Variables here
    int height;                     // Rendered image height
    point3 center;                  // Camera center
    point3 pixel00_loc;             // Location of pixel 0, 
    vec3 pixel_delta_u;             // Offset to pixel to the right
    vec3 pixel_delta_v;             // Offset to pixel below
    double pixel_samples_scale;     // Color scale factor for a sum of pixel samples -- anti-aliasing
    vec3 u, v, w;                   // Camera frame basis vectors
    vec3 defocus_disk_u;            // Defocus disk horizontal radius
    vec3 defocus_disk_v;            // Defocus disk vertical radius

    void initialize() {
        height = int(width / aspect_ratio);
        height = (height < 1) ? 1 : height;

        pixel_samples_scale = 1.0 / samples_per_pixel;

        center = lookfrom;

        // Determine viewport dimensions
        double theta = degrees_to_radians(vfov);
        double h = std::tan(theta/2);
        double viewport_height = 2 * h * focus_dist;
        double viewport_width = viewport_height * (double(width) / double(height));

        // calculate the uv,w unit basis vectors for the camera coordinate frame
        w = unit_vector(lookfrom - lookat);
        u = unit_vector(cross(vup, w));
        v = cross(w, u);

        // calculate vectors across horizontal + down vertical viewport edges
        vec3 viewport_u = viewport_width * u;       // across viewport horizontal edge
        vec3 viewport_v = viewport_height * -v;     // down vec viewport vertical edge

        // calculate horizontal + vertical delta vectors from pixel to pixel
        pixel_delta_u = viewport_u / width;
        pixel_delta_v = viewport_v / height;

        // calculate location of upper left pixel (0th pixel)
        auto viewport_upper_left = center - (focus_dist * w) - viewport_u/2 - viewport_v/2;
        pixel00_loc = viewport_upper_left + 0.5 * (pixel_delta_u + pixel_delta_v);

        // Calcualt ethe camera defocus disk basis vectors
        auto defocus_radius = focus_dist * std::tan(degrees_to_radians(defocus_angle / 2));
        defocus_disk_u = u * defocus_radius;
        defocus_disk_v = v * defocus_radius;

    }

    ray get_ray(int i, int j) const {
        // Constructs a camera ray originating from the origin and directed at randomly sampled
        // point around the pixel location i, j

        auto offset = sample_square();
        auto pixel_sample = pixel00_loc + ((i + offset.x()) * pixel_delta_u) + ((j + offset.y()) * pixel_delta_v);

        // camera with adjustable depth of field // focus range
        auto ray_origin = (defocus_angle <= 0) ? center : defocus_disk_sample();
        auto ray_direction = pixel_sample - ray_origin;

        return ray(ray_origin, ray_direction);
    }

    vec3 sample_square() const {
        // Returns the vector to a random point in teh [+-.5, +-.5] unit square range
        return vec3(random_double() - 0.5, random_double() + 0.5, 0);
    }

    point3 defocus_disk_sample() const {
        // returns a random point inside of camera defocus disk
        auto p = random_in_unit_disk();
        return center + (p[0] * defocus_disk_u) + (p[1] * defocus_disk_v);
    }

    color ray_color(const ray& r, int depth, const hittable& world) const {
        if (depth <= 0) {
            return color(0, 0, 0);
        }

        hit_record rec;
        if (world.hit(r, interval(0.001, infinity), rec)) {             // the 0.001 fixes shadow acne
            ray scattered;
            color attenuation;

            // scatter has valid calculations
            if (rec.mat -> scatter(r, rec, attenuation, scattered)){
                // calculate loss of color by reflection
                return attenuation * ray_color(scattered, depth-1, world);
            }
            return color(0, 0, 0);
        }

        vec3 unit_direction = unit_vector(r.direction());
        auto a = 0.5 * (unit_direction.y() + 1.0);
        return (1.0-a) * color(1.0, 1.0, 1.0) + a*color(0.5, 0.7, 1.0);
    }

public:
    double aspect_ratio = 1.0;      // ratio of width over height
    int width = 100;                // rendered image width in pixel count 
    int samples_per_pixel = 10;     // Count of random samples for each pixel
    int max_depth = 10;             // Max number of ray bounces into scene

    double vfov         = 90;                   // vertical fov
    point3 lookfrom     = point3(0,0,0);        // point camera is located
    point3 lookat       = point3(0, 0, -1);     // point camera is looking at
    vec3 vup            = vec3(0, 1, 0);        // camera relative "up" vec3

    double defocus_angle = 0;           // variation angle of rays through each pixel
    double focus_dist = 10;             // distance from camera lookfrom point to plane of perfect focus
                                        // everything before plane == perfect focus
                                        // everything past plane   == defocused!!

    void render(const hittable& world, int min_width, int max_width) {   
        initialize();

        time_t start_time = time(NULL);

        std::ofstream output("assets/output-no-multi-proc.ppm");
        
        output << "P3" << std::endl << width << ' ' << height << std::endl << 255 << std::endl;

        double r, g, b;
        int ir, ig, ib;
        for (int j = 0; j < height; j++) {
            // log the progress
            std::clog << "\rScanlines remaining: " << (height - j) << " " << std::flush;

            for (int i = min_width; i < max_width; i++) {
                color pixel_color(0, 0, 0);

                // anti-aliasing
                for (int sample = 0; sample < samples_per_pixel; sample++) {
                    ray r = get_ray(i, j);
                    pixel_color += ray_color(r, max_depth, world);
                }
                write_color(output, pixel_samples_scale * pixel_color);
            }
        }
        std::clog << "\rDone.               \n";

        time_t end_time = time(NULL);
        std::cout << "Time taken: " << (end_time - start_time) << " seconds" << std::endl;
    }

    bool multi_process_render(const hittable_list* world, int min_width, int max_width) {
        initialize();

        std::chrono::steady_clock::time_point start_time = std::chrono::steady_clock::now();

        // Goal: max # of processes = (max # for computer)
        // Stage 1.1: groundwork
        // int process_count = sysconf(_SC_NPROCESSORS_ONLN) - 1;
        const int process_count = 7; 
        if (process_count < 0) { // less than 1 core
            std::cerr << "Error: invalid number of processes" << std::endl;
            return false;
        }
        int valid_threads = 0;
        
        pipe_file_directory pipefd[process_count];
        pid_t children[process_count];
        std::ofstream files[process_count];
        std::mutex cout_mutex;

        for (int i = 0; i < process_count; i++) {
            files[i].open("assets/output" + std::to_string(i) + ".chk", std::ios::out | std::ios::trunc);
            if (!files[i].is_open()) {
                std::cerr << "Error: file failed to open" << std::endl;
                return false;
            }
        }

        // Stage 1.2: create pipes and batch children
        std::cout << "Creating Child Processes" << std::endl;
        for (int i = 0; i < process_count; i++) {
            // create pipes
            int tpipefd[2];
            if (pipe(tpipefd) == -1) {
                std::cerr << "Error: pipe failed" << std::endl;
                return false;
            }
            pipefd[i] = {tpipefd[0], tpipefd[1]};

            // fork child processes
            
            min_width = (width / process_count) * i;
            max_width = (i == process_count - 1) ? width : (width / process_count) * (i + 1);

            pid_t pid = fork();
            if (pid < 0) {
                // close child pipes
                close(pipefd[i].read);
                close(pipefd[i].write);
                // flag child as failed
                children[i] = -1;
                continue;
            } else if (pid == 0) {          // active child script
                // close reading pipe -- child does not read
                close(pipefd[i].read);

                // Stage 2.1: open file io -- write to file
                // Child: i | OPEN
                const char* message = "OPEN";
                write(pipefd[i].write, message, strlen(message) + 1);

                // Stage 2.2: render section
                std::cout << "Rendering in child process: " << i << std::endl;
                
                // begin rendering
                area2d area = {min_width, max_width, 0, height};
                render_portion(world, &area, &pipefd[i], &files[i]);

                // close file
                files[i].close();

                // Stage 2.3: write to pipe when finished
                // Child: i | FINISHED
                message = "FINISHED";
                write(pipefd[i].write, message, strlen(message) + 1);

                std::chrono::steady_clock::time_point end_time = std::chrono::steady_clock::now();

                message = ("TIME " + std::to_string(std::chrono::duration_cast<std::chrono::seconds>(end_time - start_time).count())).c_str();
                write(pipefd[i].write, message, strlen(message) + 1);

                // Stage 2.4: clean
                // close write pipe
                close(pipefd[i].write);
                exit(0);

            } else {                        // active parent script
                // TODO _ potentially get the parent to all 
                // perform calculations
                children[i] = pid;
                valid_threads ++;

                std::cout << "Created Child: " << i << std::endl;
            }
        }

        // Stage 3: run parent script info
        //      - reads from the pipes of child processes
        //      - stores data into a buffer
        //      - writes to the .ppm file (at the very end)
        
        std::cout << "Stage 3 -- Starting to read from child processes" << std::endl;

        // Stage 3.1: check if valid resources
        if (valid_threads <= 0) {
            std::cerr << "Error: invalid thread count" << std::endl;
            return false;
        }

        // Stage 3.2: read from pipes
        std::atomic<int> lines_rendered(0);
        // TODO - implement threading to read from all pipes
        std::vector<std::thread> readers;
        readers.reserve(process_count);

        

        for (int i = 0; i < process_count; i++) {
            if (children[i] > 0) {      // if is valid child process
                
                // launch a thread to read
                readers.emplace_back([this, i, &pipefd, &lines_rendered, &cout_mutex]() {
                    char buffer[256];

                    // continuously read until EOF or error
                    while (true) {
                        ssize_t bytes_read = read(pipefd[i].read, buffer, sizeof(buffer) - 1);
                        if (bytes_read <= 0) {
                            // bytes_read == 0 -- means it is EOF
                            // bytes_read < 0 -- means there was an error
                            break;
                        }

                        // nbull terminated and print or process the data
                        buffer[bytes_read] = '\0';

                        // check if a certain prefix is present
                        if (strncmp(buffer, "OPEN", 4) == 0 || strncmp(buffer, "FINISHED", 8) == 0) {
                            std::lock_guard<std::mutex> lock(cout_mutex);
                            std::cout << "Child " << i << " says | OPEN" << std::endl;
                        } else if (strncmp(buffer, "UPDATE", 6) == 0) {
                            lines_rendered.fetch_add(1);
                            {
                                std::lock_guard<std::mutex> lock(cout_mutex);
                                std::clog << "\rScanlines remaining: " << (height * process_count - lines_rendered.load()) << " " << std::flush;
                            }
                        } else if(strncmp(buffer, "TIME", 4) == 0) {
                            std::lock_guard<std::mutex> lock(cout_mutex);
                            std::cout << "Child " << i << " says | " << buffer << std::endl;
                        }else {
                            std::lock_guard<std::mutex> lock(cout_mutex);
                            std::cout << "Child " << i << " says | " << buffer << std::endl;
                        }
                    }
                    
                });
            }
        }

        // Stage 3.3: clean
        // close processes + pipes
        std::cout << "Closing Child Processes" << std::endl;
        for (int i = 0; i < process_count; i++) {
            if (children[i] > 0) {
                waitpid(children[i], NULL, 0);
                close(pipefd[i].read);
                close(pipefd[i].write);

                std::cout << "Closed Child: " << i << std::endl;
            }
        }

        // join the threads
        for (auto& reader : readers) {
            reader.join();
        }

        // Stage 3.4: Write to file
        std::ifstream files_read[process_count];
        int* buffer = new int[height * width * 3];

        std::ofstream log_file("assets/output.log", std::ios::out | std::ios::trunc);
        if (!log_file.is_open()) {
            std::cerr << "Error: log file failed to open" << std::endl;
            return false;
        }

        // read each file and load data into the buffer ( RAW FILE )
        const char* base_dir = "assets/output";
        for (int i = 0; i < process_count; i++) {
            files_read[i].open(base_dir + std::to_string(i) + ".chk");
            
            // check if file opened
            if (!files_read[i].is_open()) {
                std::cerr << "Error: file failed to open" << std::endl;
                return false;
            }
        }

        // open the output file
        std::ofstream output("assets/output-w-multi-proc.ppm");
        if (!output.is_open()) {
            std::cerr << "Error: output file failed to open" << std::endl;
            return false;
        }

        // dump image buffer into the output file
        output << "P3" << std::endl << width << ' ' << height << std::endl << 255 << std::endl;

        std::pair<int, int> sizes[process_count];
        std::string tmp;
        for (int i = 0; i < process_count; i++) {
            getline(files_read[i], tmp);
            // width + height
            getline(files_read[i], tmp);
            std::stringstream ss(tmp);
            ss >> sizes[i].first >> sizes[i].second;
            // format
            getline(files_read[i], tmp);
            // TODO - do something with that data
        }

        // all chk files read as --> (pixel, pixel, pixel on every line [from left to right, top to bottom])
        for (int y = 0; y < height; y++) {
            for (int i = 0; i < process_count; i++) {
                int r, g, b;

                for (int x = 0; x < sizes[i].first; x++) {
                    files_read[i] >> r >> g >> b;
                    output << r << ' ' << g << ' ' << b << std::endl;
                }
            }
        }


        // close all files
        for (int i = 0; i < process_count; i++) {
            files_read[i].close();
        }
        output.close();

        std::clog << "\rDone.               \n";

        return true;

    }

    void render_portion(const hittable* world, area2d *portion, pipe_file_directory *pipefd, std::ofstream *file) {
        // camera already initialized
        // header data already outputted
        
        double r, g, b;
        int ir, ig, ib;

        *file << "P3" << std::endl << (portion->max_x - portion->min_x) << ' ' << (portion->max_y - portion->min_y) << std::endl << 255 << std::endl;

        for(int y = portion->min_y; y < portion->max_y; y++) {
            // TODO -- log process 
            const char* message = "UPDATE";
            write(pipefd->write, message, strlen(message) + 1);

            for (int x = portion->min_x; x < portion->max_x; x++) {

                color pixel_color(0, 0, 0);
                // aa
                for(int sample = 0; sample < samples_per_pixel; sample++) {
                    ray r = get_ray(x, y);
                    pixel_color += ray_color(r, max_depth, *world);
                }

                // write to ostream file
                // *file << x << ' ' << y << ' ';
                write_color(*file, pixel_samples_scale * pixel_color);

            }
        }
    }

    // getters + setters
    int get_height() { return height; }

    point3 get_center() { return center; }

};

#endif