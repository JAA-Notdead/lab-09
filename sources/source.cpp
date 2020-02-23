// Copyright 2018 Your Name <your_email>

#include <header.hpp>

using boost::program_options::options_description;
using boost::program_options::variables_map;
using boost::program_options::error;
using boost::program_options::store;
using boost::program_options::notify;
using boost::program_options::value;

int main(int argc, char* argv[]){

    try {
        options_description desc{"Options"};
        desc.add_options()
                ("url",
                        value<std::string>()->required(),
                        "url")
                ("depth",
                        value<int>()->required(),
                        "depth")
                ("network_threads",
                        value<int>()->default_value(1),
                        "network thread count")
                ("parser_threads",
                        value<int>()->default_value(1),
                        "parser thread count")
                ("output",
                        value<std::string>()->required(),
                        "output file path");

        variables_map vm;
        store(parse_command_line(argc, argv, desc), vm);
        notify(vm);
        io_context ioContext;
        tcp::resolver resolver(ioContext);
        tcp_stream stream(ioContext);
        const auto url = vm["url"].as<std::string>();
        auto const port = (url.substr(4, 1) == "s") ? "443": "80";
        auto const results = resolver.resolve(url, port);
        stream.connect(results);
        std::string target;
        std::string host;
        if (std::count(
                url.begin(),
                url.end(),
                '/') > 3) {
            auto const begin = (
                    url.substr(4, 1) == "s")
                            ? 8
                            : 7;
            auto end = url.find('\\', begin);
            host = url.substr(begin, end - begin);
            target = url.substr(end);
        } else {
            auto const begin = (url.substr(4, 1) == "s") ? 8: 7;
            host = url.substr(begin);
            target = "/";
        }
        request<string_body> req{verb::get, target, 10};
        req.set(field::host, host);
        req.set(field::user_agent, BOOST_BEAST_VERSION_STRING);
        write(stream, req);
        flat_buffer buffer;
        response<dynamic_body> res;
        read(stream, buffer, res);
        boost::beast::error_code ec;
        stream.socket().shutdown(tcp::socket::shutdown_both, ec);
        if (ec && ec != boost::beast::errc::not_connected) {
            throw boost::beast::system_error(ec);
        }
    } catch (const error& ex){
        std::cerr << ex.what() << std::endl;
    }
}