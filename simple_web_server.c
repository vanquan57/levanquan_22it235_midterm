#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <fcntl.h>

#define PORT 9090
#define BUFFER_SIZE 1024

// Hàm để đọc file và trả về nội dung
char *read_file(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *content = malloc(length + 1);
    if (content) {
        fread(content, 1, length, file);
        content[length] = '\0';
    }

    fclose(file);
    return content;
}

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    char buffer[BUFFER_SIZE] = {0};
    
    // Tạo socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Thiết lập địa chỉ IP và cổng
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Gán socket với cổng đã chọn
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    // Bắt đầu lắng nghe kết nối
    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    printf("Server le van quan is running on port %d\n", PORT);

    while (1) {
        // Chấp nhận kết nối từ client
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
            perror("accept");
            exit(EXIT_FAILURE);
        }

        // Đọc yêu cầu từ client
        read(new_socket, buffer, BUFFER_SIZE);
        printf("%s\n", buffer);

        // Xác định loại yêu cầu từ client
        char *file_to_send = NULL;
        if (strncmp(buffer, "GET /styles.css", 15) == 0) {
            file_to_send = "styles.css";  // Tệp CSS nằm cùng cấp
        } else {
            file_to_send = "index.html";  // Tệp HTML nằm cùng cấp
        }

        // Đọc nội dung file
        char *content = read_file(file_to_send);
        if (content) {
            // Xây dựng tiêu đề HTTP cho phản hồi
            char response[BUFFER_SIZE];
            snprintf(response, sizeof(response),
                     "HTTP/1.1 200 OK\n"
                     "Content-Type: %s\n"
                     "Content-Length: %ld\n\n%s",
                     strstr(file_to_send, ".css") ? "text/css" : "text/html",
                     strlen(content),
                     content);

            // Gửi nội dung file và tiêu đề
            send(new_socket, response, strlen(response), 0);
            free(content);
        } else {
            // Gửi thông báo lỗi nếu không tìm thấy file
            const char *error_message = " Lê Văn Quân 22it235- HTTP/1.1 404 Not Found\nContent-Length: 13\n\n404 Not Found";
            send(new_socket, error_message, strlen(error_message), 0);
        }

        // Đóng kết nối
        close(new_socket);
    }

    close(server_fd);
    return 0;
}
