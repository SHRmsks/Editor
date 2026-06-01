import type { Metadata } from "next";
import "./global.css";

export const metadata: Metadata = {
  title: "Online Editor",
  description: "Online Rich Text Editor",
};

export default function RootLayout({
  children,
}: Readonly<{
  children: React.ReactNode;
}>) {
  return (
    <html lang="en">
      <body className={`antialiased`}>{children}</body>
    </html>
  );
}
